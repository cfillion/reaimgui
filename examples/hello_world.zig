// zig build-lib -dynamic -O ReleaseFast -femit-bin=reaper_zig.so hello_world.zig

const std = @import("std");
const ImGui = @import("reaper_imgui");
const reaper = struct { // @import("reaper");
  pub const PLUGIN_VERSION = 0x20E;

  pub const HINSTANCE      = *opaque {};
  pub const HWND           = *opaque {};
  pub const KbdSectionInfo =  opaque {};

  pub const plugin_info_t = extern struct {
    caller_version: c_int,
    hwnd_main: HWND,
    register: ?@TypeOf(plugin_register),
    getFunc:  ?@TypeOf(plugin_getapi),
  };

  pub const custom_action_register_t = extern struct {
    section: c_int,
    id_str: [*:0]const u8,
    name:   [*:0]const u8,
    extra:  ?*anyopaque = null,
  };

  pub fn init(rec: *plugin_info_t) bool {
    if(rec.caller_version != PLUGIN_VERSION) {
      std.debug.print("expected REAPER API version {x}, got {x}\n",
        .{ PLUGIN_VERSION, rec.caller_version });
      return false;
    }

    const getFunc = rec.getFunc.?;
    inline for(@typeInfo(@This()).@"struct".decls) |decl| {
      comptime var decl_type = @typeInfo(@TypeOf(@field(@This(), decl.name)));
      const is_optional = decl_type == .optional;
      if(is_optional)
        decl_type = @typeInfo(decl_type.optional.child);
      if(decl_type != .pointer or @typeInfo(decl_type.pointer.child) != .@"fn")
        continue;
      if(getFunc(decl.name)) |func|
        @field(@This(), decl.name) = @ptrCast(func)
      else if(is_optional)
        @field(@This(), decl.name) = null
      else {
        std.debug.print("unable to import the API function '{s}'\n", .{ decl.name });
        return false;
      }
    }

    return true;
  }

  pub var plugin_register: *fn(name: [*:0]const u8, infostruct: *anyopaque) callconv(.C) c_int = undefined;
  pub var plugin_getapi:   *fn(name: [*:0]const u8) callconv(.C) ?*anyopaque = undefined;
  pub var ShowMessageBox:  *fn(body: [*:0]const u8, title: [*:0]const u8, flags: c_int) callconv(.C) void = undefined;
};

const plugin_name = "Hello, Zig!";
var action_id: c_int = undefined;
var ctx: ImGui.ContextPtr = null;
var click_count: u32 = 0;
var text = std.mem.zeroes([255:0]u8);

fn loop() !void {
  if(ctx == null) {
    try ImGui.init(reaper.plugin_getapi);
    ctx = try ImGui.CreateContext(.{ plugin_name });
  }

  try ImGui.SetNextWindowSize(.{ ctx, 400, 80, ImGui.Cond_FirstUseEver });

  var open: bool = true;
  if(try ImGui.Begin(.{ ctx, plugin_name, &open })) {
    if(try ImGui.Button(.{ ctx, "Click me!" }))
      click_count +%= 1;

    if(click_count & 1 != 0) {
      try ImGui.SameLine(.{ ctx });
      try ImGui.Text(.{ ctx, "\\o/" });
    }

    _ = try ImGui.InputText(.{ ctx, "text input", &text, text.len });
    try ImGui.End(.{ ctx });
  }

  if(!open)
    reset();
}

fn init() void {
  _ = reaper.plugin_register("timer", @constCast(@ptrCast(&onTimer)));
}

fn reset() void {
  _ = reaper.plugin_register("-timer", @constCast(@ptrCast(&onTimer)));
  ctx = null;
}

fn onTimer() callconv(.C) void {
  loop() catch {
    reset();
    reaper.ShowMessageBox(ImGui.last_error.?, plugin_name, 0);
  };
}

fn onCommand(sec: *reaper.KbdSectionInfo, command: c_int, val: c_int,
  val2hw: c_int, relmode: c_int, hwnd: reaper.HWND) callconv(.C) c_char
{
  _ = .{ sec, val, val2hw, relmode, hwnd };

  if(command == action_id) {
    if(ctx == null) init() else reset();
    return 1;
  }

  return 0;
}

export fn ReaperPluginEntry(instance: reaper.HINSTANCE, rec: ?*reaper.plugin_info_t) c_int {
  _ = instance;

  if(rec == null)
    return 0 // cleanup here
  else if(!reaper.init(rec.?))
    return 0;

  const action = reaper.custom_action_register_t
    { .section = 0, .id_str = "REAIMGUI_ZIG", .name = "ReaImGui Zig example" };
  action_id = reaper.plugin_register("custom_action", @constCast(@ptrCast(&action)));
  _ = reaper.plugin_register("hookcommand2", @constCast(@ptrCast(&onCommand)));

  return 1;
}
