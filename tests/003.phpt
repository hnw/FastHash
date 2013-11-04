--TEST--
Check for behavior with subclass of FastHash
--SKIPIF--
<?php if (!extension_loaded("fasthash")) print "skip"; ?>
--FILE--
<?php 
class FastHashChild extends Fasthash
{
  function offsetGet($index)
  {
    $ret = parent::offsetGet($index);
    return (int)$ret;
  }
}
$hash = new FastHashChild();
$hash[10] = 20;
var_dump($hash[10]);
--EXPECT--
int(20)
