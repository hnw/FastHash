dnl $Id$
dnl config.m4 for extension fasthash

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(fasthash, for fasthash support,
dnl Make sure that the comment is aligned:
dnl [  --with-fasthash             Include fasthash support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(fasthash, whether to enable fasthash support,
Make sure that the comment is aligned:
[  --enable-fasthash       Enable fasthash support])

if test "$PHP_FASTHASH" != "no"; then
  PHP_ARG_WITH(glib2,for glib2 support,
  [  --with-glib2[=DIR]        Include glib2 support])
  
  if test "$PHP_GLIB2" && test "$PHP_GLIB2" != "no"; then
    AC_MSG_CHECKING(for glib-2.0 support)
    if test -z "$PKG_CONFIG"; then
      AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
    fi
    if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists glib-2.0; then
      FASTHASH_INCLUDE=`$PKG_CONFIG glib-2.0 --cflags-only-I`
      FASTHASH_LFLAGS=`$PKG_CONFIG glib-2.0 --libs-only-L`
      FASTHASH_LIBS=`$PKG_CONFIG glib-2.0 --libs-only-l`
      PHP_EVAL_INCLINE($FASTHASH_INCLUDE)
      PHP_EVAL_LIBLINE([$FASTHASH_LFLAGS $FASTHASH_LIBS], FASTHASH_SHARED_LIBADD)
      AC_DEFINE(HAVE_GLIB2,1,[ ])
    fi
  fi

  PHP_SUBST(FASTHASH_SHARED_LIBADD)

  PHP_NEW_EXTENSION(fasthash, fasthash.c, $ext_shared)
fi
