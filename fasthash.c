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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_fasthash.h"


#include "Zend/zend_interfaces.h"
#include "spl/spl_iterators.h"

#include "Zend/zend_exceptions.h"
#include "spl/spl_exceptions.h"

#include "glib.h"

/* If you declare any globals in php_fasthash.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(fasthash)
*/

/* True global resources - no need for thread safety here */
static int le_fasthash;

typedef GHashTable     ht;
typedef GHashTableIter ht_iter;

typedef struct _fasthash_object {
	zend_object         std;
	ht                  *ht;
} fasthash_object;

typedef struct _fasthash_iterator {
	zend_object_iterator intern;
	fasthash_object *fasthash;
	ht_iter *ht_iter;
	char *key;
	zval *current;
} fasthash_iterator;

void key_destroy_func(gpointer p_key)
{
	efree(p_key);
}

void value_destroy_func(gpointer p_value)
{
	zval_ptr_dtor((zval **)&p_value);
}

int ht_init(fasthash_object *intern)
{
	intern->ht = g_hash_table_new_full(g_str_hash, g_str_equal, &key_destroy_func, &value_destroy_func);
	return 0;
}

int ht_destroy(fasthash_object *intern)
{
	g_hash_table_destroy(intern->ht);
	return 0;
}

int ht_insert(fasthash_object *intern, char *p_key, zval *zp)
{
	Z_ADDREF_P(zp);
	g_hash_table_insert(intern->ht, estrdup(p_key), zp);
	return 0;
}

zval *ht_get(fasthash_object *intern, char *p_key)
{
	return g_hash_table_lookup(intern->ht, p_key);
}

int ht_remove(fasthash_object *intern, char *p_key)
{
	return g_hash_table_remove(intern->ht, p_key) ? 0 : -1;
}

int ht_iter_init(fasthash_object *intern, ht_iter *ht_iter)
{
	g_hash_table_iter_init(ht_iter, intern->ht);

	return 0;
}

int ht_iter_next(ht_iter *ht_iter, char **pp_key, zval **pp_value)
{
	int ret = g_hash_table_iter_next(ht_iter, (void **)pp_key, (void **)pp_value);
	return ret;
}

zend_object_handlers fasthash_object_handlers;
PHPAPI zend_class_entry  *ce_FastHash;

ZEND_BEGIN_ARG_INFO_EX(arginfo_fasthash_offsetGet, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fasthash_offsetSet, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, newval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fasthash_unserialize, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_fasthash_void, 0)
ZEND_END_ARG_INFO()

/* {{{ fasthash_functions[]
 *
 * Every user visible function must have an entry in fasthash_functions[].
 */
const zend_function_entry fasthash_functions[] = {
	PHP_ME(FastHash, __construct,  arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	/* Countable */
	PHP_ME(FastHash, count,        arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	/* Iterator */
	PHP_ME(FastHash, rewind,       arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, current,      arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, key,          arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, next,         arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, valid,        arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	/* ArrayAccess */
	PHP_ME(FastHash, offsetExists, arginfo_fasthash_offsetGet,   ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, offsetGet,    arginfo_fasthash_offsetGet,   ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, offsetSet,    arginfo_fasthash_offsetSet,   ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, offsetUnset,  arginfo_fasthash_offsetGet,   ZEND_ACC_PUBLIC)
	/* Serializable */
	PHP_ME(FastHash, serialize,    arginfo_fasthash_void,        ZEND_ACC_PUBLIC)
	PHP_ME(FastHash, unserialize,  arginfo_fasthash_unserialize, ZEND_ACC_PUBLIC)
	PHP_FE_END	/* Must be the last line in fasthash_functions[] */
};

/* }}} */

/* {{{ fasthash_module_entry
 */
zend_module_entry fasthash_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"fasthash",
	NULL,
	PHP_MINIT(fasthash),
	PHP_MSHUTDOWN(fasthash),
	PHP_RINIT(fasthash),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(fasthash),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(fasthash),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_FASTHASH_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FASTHASH
ZEND_GET_MODULE(fasthash)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("fasthash.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_fasthash_globals, fasthash_globals)
    STD_PHP_INI_ENTRY("fasthash.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_fasthash_globals, fasthash_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_fasthash_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_fasthash_init_globals(zend_fasthash_globals *fasthash_globals)
{
	fasthash_globals->global_value = 0;
	fasthash_globals->global_string = NULL;
}
*/
/* }}} */

static void fasthash_free_object_storage(fasthash_object *intern TSRMLS_DC) /* {{{ */
{
	ht_destroy(intern);

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	efree(intern);
}
/* }}} */

zend_object_value fasthash_create_object(zend_class_entry *class_type TSRMLS_DC)  /* {{{ */
{
	zend_object_value retval;
	fasthash_object *intern;

	intern = emalloc(sizeof(fasthash_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	ht_init(intern);

	retval.handlers = &fasthash_object_handlers;

	retval.handle = zend_objects_store_put(intern,
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)fasthash_free_object_storage,
		 NULL TSRMLS_CC
	);

	return retval;
}
/* }}} */

zend_object_value fasthash_clone(zval *object TSRMLS_DC)  /* {{{ */
{
	fasthash_object *old_object = zend_object_store_get_object(object TSRMLS_CC);
	zend_object_value new_object_val = fasthash_create_object(Z_OBJCE_P(object) TSRMLS_CC);
	fasthash_object *new_object = zend_object_store_get_object_by_handle(
		new_object_val.handle TSRMLS_CC
	);

	zend_objects_clone_members(
		&new_object->std, new_object_val,
		&old_object->std, Z_OBJ_HANDLE_P(object) TSRMLS_CC
	);

	{
		ht_iter iter;
		char *key;
		zval *pz;

		ht_iter_init(old_object, &iter);
		while (ht_iter_next(&iter, &key, &pz)) {
			ht_insert(new_object, key, pz);
		}
	}

    return new_object_val;
}
/* }}} */

static inline zval *fasthash_object_read_dimension_helper(fasthash_object *intern, zval *offset TSRMLS_DC) /* {{{ */
{
	zval key, *key_ptr = offset, *value_ptr;

	/* we have to return NULL on error here to avoid memleak because of 
	 * ZE duplicating uninitialized_zval_ptr */
	if (!offset) {
		zend_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0 TSRMLS_CC);
		return NULL;
	}

	if (Z_TYPE_P(offset) != IS_STRING) {
		key = *offset;
		zval_copy_ctor(&key);
		convert_to_string(&key);
		key_ptr = &key;
	}

	value_ptr = ht_get(intern, Z_STRVAL_P(key_ptr));

	if (key_ptr != offset) {
		zval_dtor(&key);
	}

	return value_ptr;
}
/* }}} */

static zval *fasthash_object_read_dimension(zval *object, zval *offset, int type TSRMLS_DC) /* {{{ */
{
	fasthash_object *intern;
	zval *retval;

	intern = (fasthash_object *)zend_object_store_get_object(object TSRMLS_CC);

	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->read_dimension(object, offset, type TSRMLS_CC);
	}

	retval = fasthash_object_read_dimension_helper(intern, offset TSRMLS_CC);
	if (retval) {
		return retval;
	}
	return NULL;
}
/* }}} */

static inline void fasthash_object_write_dimension_helper(fasthash_object *intern, zval *offset, zval *value TSRMLS_DC) /* {{{ */
{
	zval key, *key_ptr = offset;

	if (!offset) {
		/* '$array[] = value' syntax is not supported */
		zend_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0 TSRMLS_CC);
		return;
	}

	if (Z_TYPE_P(offset) != IS_STRING) {
		key = *offset;
		zval_copy_ctor(&key);
		convert_to_string(&key);
		key_ptr = &key;
	}

	ht_insert(intern, Z_STRVAL_P(key_ptr), value);

	if (key_ptr != offset) {
		zval_dtor(&key);
	}
}
/* }}} */

static void fasthash_object_write_dimension(zval *object, zval *offset, zval *value TSRMLS_DC) /* {{{ */
{
	fasthash_object *intern;

	intern = (fasthash_object *)zend_object_store_get_object(object TSRMLS_CC);

	fasthash_object_write_dimension_helper(intern, offset, value TSRMLS_CC);
}

static void fasthash_iterator_dtor(zend_object_iterator *intern TSRMLS_DC)
{
	fasthash_iterator *iter = (fasthash_iterator *)intern;

	if (iter->current) {
		zval_ptr_dtor(&iter->current);
	}

	zval_ptr_dtor((zval **) &intern->data);
	efree(iter->ht_iter);
	efree(iter);
}

static int fasthash_iterator_valid(zend_object_iterator *intern TSRMLS_DC)
{
    fasthash_iterator *iter = (fasthash_iterator *)intern;

    return iter->current ? SUCCESS : FAILURE;
}

static void fasthash_iterator_get_current_data(zend_object_iterator *intern, zval ***data TSRMLS_DC)
{
	fasthash_iterator *iter = (fasthash_iterator *)intern;

	if (iter->current == NULL) {
		*data = &EG(uninitialized_zval_ptr);
	} else {
		*data = &iter->current;
	}
}

#if ZEND_MODULE_API_NO >= 20121212
/* for PHP 5.5.x or newer */
static void fasthash_iterator_get_current_key(zend_object_iterator *intern, zval *key TSRMLS_DC)
{
	fasthash_iterator *iter = (fasthash_iterator *) intern;
	if (iter->key == NULL) {
		zend_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0 TSRMLS_CC);
	}
	ZVAL_STRING(key, iter->key, 1);
}
#else
/* for PHP 5.4.x or older */
static int fasthash_iterator_get_current_key(zend_object_iterator *intern, char **str_key, uint *str_key_len, ulong *int_key TSRMLS_DC)
{
	fasthash_iterator *iter = (fasthash_iterator *) intern;
	if (iter->key == NULL) {
		zend_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0 TSRMLS_CC);
	}
	*str_key = estrdup(iter->key);
	*str_key_len = strlen(*str_key) + 1;
	return HASH_KEY_IS_STRING;
}
#endif

static void fasthash_iterator_move_forward(zend_object_iterator *intern TSRMLS_DC)
{
	fasthash_iterator *iter = (fasthash_iterator *)intern;
	int ret;

	if (!ht_iter_next(iter->ht_iter, &iter->key, &iter->current)) {
		iter->key = NULL;
		iter->current = NULL;
	}
}

static void fasthash_iterator_rewind(zend_object_iterator *intern TSRMLS_DC)
{
    fasthash_iterator *iter = (fasthash_iterator *) intern;

	ht_iter_init(iter->fasthash, iter->ht_iter);
	if (!ht_iter_next(iter->ht_iter, &iter->key, &iter->current)) {
		iter->key = NULL;
		iter->current = NULL;
	}
}

static zend_object_iterator_funcs fasthash_iterator_funcs = {
    fasthash_iterator_dtor,
    fasthash_iterator_valid,
    fasthash_iterator_get_current_data,
    fasthash_iterator_get_current_key,
    fasthash_iterator_move_forward,
    fasthash_iterator_rewind
};

zend_object_iterator *fasthash_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC)
{
	fasthash_object *intern;
	fasthash_iterator *iter;

	if (by_ref) {
		zend_throw_exception(NULL, "Cannot iterate fasthash by reference", 0 TSRMLS_CC);
		return NULL;
	}

	iter = emalloc(sizeof(fasthash_iterator));
	iter->intern.funcs = &fasthash_iterator_funcs;

	iter->intern.data = object;
	Z_ADDREF_P(object);

	iter->fasthash = (fasthash_object *)zend_object_store_get_object(object TSRMLS_CC);

	iter->ht_iter = emalloc(sizeof(ht_iter));

	iter->key = NULL;
	iter->current = NULL;

	return (zend_object_iterator *)iter;
}

/* {{{ proto void FastHash::__construct()
   Create an FastHash */
PHP_METHOD(FastHash, __construct)
{
} /* }}} */

/* {{{ proto int FastHash::count()
   Return the number of elements in the Iterator. */
PHP_METHOD(FastHash, count)
{
} /* }}} */

/* {{{ proto void FastHash::rewind()
   Rewind array back to the start */
PHP_METHOD(FastHash, rewind)
{
} /* }}} */

/* {{{ proto mixed|NULL FastHash::current()
   Return current array entry */
PHP_METHOD(FastHash, current)
{
} /* }}} */

/* {{{ proto mixed|NULL FastHash::key()
   Return current array key */
PHP_METHOD(FastHash, key)
{
}

/* {{{ proto void FastHash::next()
   Move to next entry */
PHP_METHOD(FastHash, next)
{
}

/* {{{ proto bool FastHash::valid()
   Check whether array contains more entries */
PHP_METHOD(FastHash, valid)
{
}

/* {{{ proto bool FastHash::offsetExists(mixed $index)
   Returns whether the requested $index exists. */
PHP_METHOD(FastHash, offsetExists)
{
	printf("offsetexists\n");
} /* }}} */

/* {{{ proto mixed FastHash::offsetGet(mixed $index)
   Returns the value at the specified $index. */
PHP_METHOD(FastHash, offsetGet)
{
	zval            *zindex, *value_p;
	fasthash_object *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zindex) == FAILURE) {
		return;
	}
	intern    = (fasthash_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	value_p  = fasthash_object_read_dimension_helper(intern, zindex TSRMLS_CC);

	if (value_p) {
		RETURN_ZVAL(value_p, 1, 0);
	}
	RETURN_NULL();
} /* }}} */

/* {{{ proto void FastHash::offsetSet(mixed $index, mixed $newval)
   Sets the value at the specified $index to $newval. */
PHP_METHOD(FastHash, offsetSet)
{
	zval             *zindex, *value;
	fasthash_object  *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &zindex, &value) == FAILURE) {
		return;
	}

	intern = (fasthash_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	fasthash_object_write_dimension_helper(intern, zindex, value TSRMLS_CC);

} /* }}} */

/* {{{ proto void FastHash::offsetUnset(mixed $index)
   Unsets the value at the specified $index. */
PHP_METHOD(FastHash, offsetUnset)
{
	zval            *zindex;
	fasthash_object *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zindex) == FAILURE) {
		return;
	}
	intern    = (fasthash_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	convert_to_string(zindex);

	ht_remove(intern, Z_STRVAL_P(zindex));
} /* }}} */

/* {{{ proto string FastHash::serialize()
   Serialize the object */
PHP_METHOD(FastHash, serialize)
{
} /* }}} */

/* {{{ proto void FastHash::unserialize(string $serialized)
 * unserialize the object */
PHP_METHOD(FastHash, unserialize)
{
} /* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(fasthash)
{
	zend_class_entry tmp_ce;

	INIT_CLASS_ENTRY(tmp_ce, "FastHash", fasthash_functions);
	ce_FastHash = zend_register_internal_class(&tmp_ce TSRMLS_CC);
	ce_FastHash->create_object = fasthash_create_object;
	ce_FastHash->get_iterator = fasthash_get_iterator;
	ce_FastHash->iterator_funcs.funcs = &fasthash_iterator_funcs;

	memcpy(&fasthash_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	fasthash_object_handlers.clone_obj = fasthash_clone;
	fasthash_object_handlers.read_dimension = fasthash_object_read_dimension;
	fasthash_object_handlers.write_dimension = fasthash_object_write_dimension;

	zend_class_implements(ce_FastHash TSRMLS_CC, 4, spl_ce_Countable, zend_ce_iterator, zend_ce_arrayaccess, zend_ce_serializable);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(fasthash)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(fasthash)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(fasthash)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(fasthash)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "FastHash support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
