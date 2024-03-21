<?php
// Usage:
// include 'preprocess.php';
//
// $define = 42;
// macro('name', 'v1', 'v2=def');
// echo '<% if($v2 == "def") $v2 = "世界"; %>';
// echo 'Hello, $v1 and $v2!';
// endmacro();
//
// echo '$name(World)';
// echo '$name(Foo, Bar)';
// echo '$define';

function preprocess($output, $local_vars = []) {
  $pattern = '/\$(?<name>\w+)(?<is_call>\((?<call>((?>[^()]+)|(?2))*)\))?/';
  return preg_replace_callback($pattern, function($matches) use($local_vars) {
    $name = $matches['name'];
    $value = $local_vars[$name] ?? $GLOBALS[$name] ??
      trigger_error("\$name is not defined", E_USER_ERROR);
    if(!isset($matches['is_call']))
      return $value;
    else if(!is_callable($value))
      trigger_error("\$$name is not a macro", E_USER_ERROR);
    $call = preprocess($matches['call'], $local_vars);
    $args = preg_split('/,\s*/', $call);
    return $value(...$args);
  }, $output);
}

function macro($name, ...$_args_def) {
  array_walk($_args_def, function(&$arg) {
    preg_match('/^(\w+)(?:\s*=\s*(.+))?$/U', $arg, $matches);
    $arg = ['name' => $matches[1], 'default' => $matches[2] ?? ''];
  });
  ob_start(function($_code) use($name, $_args_def) {
    $_code = preg_replace('/(?<=<)%|%(?=>)/', '?', $_code);
    $GLOBALS[$name] = function(...$_args) use($_args_def, $_code) {
      foreach($_args_def as $_i => $_arg)
        ${$_arg['name']} = $_args[$_i] ?? $_arg['default'];
      foreach(array_diff_key($GLOBALS, get_defined_vars()) as $_name => $_value)
        global ${$_name};
      ob_start();
      eval('?>' . $_code);
      return preprocess(ltrim(ob_get_clean()), get_defined_vars());
    };
  });
}

function endmacro() { ob_end_clean(); }

register_shutdown_function(function() { echo preprocess(ob_get_clean()); });
ob_start();
