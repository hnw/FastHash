/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_FASTHASH_H
#define PHP_FASTHASH_H

extern zend_module_entry fasthash_module_entry;
#define phpext_fasthash_ptr &fasthash_module_entry

#define PHP_FASTHASH_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_FASTHASH_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FASTHASH_API __attribute__ ((visibility("default")))
#else
#	define PHP_FASTHASH_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

extern PHPAPI zend_class_entry *ce_FastHash;

PHP_MINIT_FUNCTION(fasthash);
PHP_MSHUTDOWN_FUNCTION(fasthash);
PHP_RINIT_FUNCTION(fasthash);
PHP_RSHUTDOWN_FUNCTION(fasthash);
PHP_MINFO_FUNCTION(fasthash);

PHP_METHOD(FastHash, __construct);
PHP_METHOD(FastHash, count);
PHP_METHOD(FastHash, rewind);
PHP_METHOD(FastHash, current);
PHP_METHOD(FastHash, key);
PHP_METHOD(FastHash, next);
PHP_METHOD(FastHash, valid);
PHP_METHOD(FastHash, offsetExists);
PHP_METHOD(FastHash, offsetGet);
PHP_METHOD(FastHash, offsetSet);
PHP_METHOD(FastHash, offsetUnset);
PHP_METHOD(FastHash, serialize);
PHP_METHOD(FastHash, unserialize);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(fasthash)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(fasthash)
*/

/* In every utility function you add that needs to use variables 
   in php_fasthash_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as FASTHASH_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define FASTHASH_G(v) TSRMG(fasthash_globals_id, zend_fasthash_globals *, v)
#else
#define FASTHASH_G(v) (fasthash_globals.v)
#endif

#endif	/* PHP_FASTHASH_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
