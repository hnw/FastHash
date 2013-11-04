--TEST--
Check for ArrayAccess of FastHash
--SKIPIF--
<?php if (!extension_loaded("fasthash")) print "skip"; ?>
--FILE--
<?php 
function foo($str) {
  return "foo".$str;
}
$hash = new FastHash();
$hash[10] = 20;
$hash->offsetSet(30, 40);
$hash['foo'] = 'bar';
$hash[foo('bar')] = foo('baz');

var_dump($hash['10']);
var_dump($hash['10']);
var_dump($hash->offsetGet(30));
var_dump($hash->offsetGet('30'));
var_dump($hash['foo']);
var_dump($hash['foobar']);
--EXPECT--
int(20)
int(20)
int(40)
int(40)
string(3) "bar"
string(6) "foobaz"
