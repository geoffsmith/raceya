dnl
dnl AX_CHECK_GLUT
dnl
dnl Check for GLUT.  If GLUT is found, the required compiler and linker flags
dnl are included in the output variables "GLUT_CFLAGS" and "GLUT_LIBS",
dnl respectively.  If GLUT is not found, "no_glut" is set to "yes".
dnl
dnl If the header "GL/glut.h" is found, "HAVE_GL_GLUT_H" is defined.  If the
dnl header "GLUT/glut.h" is found, HAVE_GLUT_GLUT_H is defined.  These
dnl preprocessor definitions may not be mutually exclusive.
dnl
dnl version: 2.0
dnl author: Braden McDaniel <braden@endoframe.com>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2, or (at your option)
dnl any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
dnl 02110-1301, USA.
dnl
dnl As a special exception, the you may copy, distribute and modify the
dnl configure scripts that are the output of Autoconf when processing
dnl the Macro.  You need not follow the terms of the GNU General Public
dnl License when using or distributing such scripts.
dnl
AC_DEFUN([AX_CHECK_GLUT],
[AC_REQUIRE([AX_CHECK_GLU])dnl
AC_REQUIRE([AC_PATH_XTRA])dnl

ax_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${GLU_CFLAGS} ${CPPFLAGS}"
AC_CHECK_HEADERS([GL/glut.h GLUT/glut.h])
CPPFLAGS="${ax_save_CPPFLAGS}"

GLUT_CFLAGS=${GLU_CFLAGS}
GLUT_LIBS=${GLU_LIBS}

m4_define([AX_CHECK_GLUT_PROGRAM],
          [AC_LANG_PROGRAM([[
# if HAVE_WINDOWS_H && defined(_WIN32)
#   include <windows.h>
# endif
# ifdef HAVE_GL_GLUT_H
#   include <GL/glut.h>
# elif defined(HAVE_GLUT_GLUT_H)
#   include <GLUT/glut.h>
# else
#   error no glut.h
# endif]],
                           [[glutMainLoop()]])])

#
# If X is present, assume GLUT depends on it.
#
AS_IF([test X$no_x != Xyes],
      [GLUT_LIBS="${X_PRE_LIBS} -lXmu -lXi ${X_EXTRA_LIBS} ${GLUT_LIBS}"])

AC_CACHE_CHECK([for GLUT library], [ax_cv_check_glut_libglut],
[ax_cv_check_glut_libglut="no"
AC_LANG_PUSH(C)
ax_save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${GLUT_CFLAGS} ${CPPFLAGS}"
ax_save_LIBS="${LIBS}"
LIBS=""
ax_check_libs="-lglut32 -lglut"
for ax_lib in ${ax_check_libs}; do
  AS_IF([test X$ax_compiler_ms = Xyes],
        [ax_try_lib=`echo $ax_lib | sed -e 's/^-l//' -e 's/$/.lib/'`],
        [ax_try_lib="${ax_lib}"])
  LIBS="${ax_try_lib} ${GLUT_LIBS} ${ax_save_LIBS}"
  AC_LINK_IFELSE([AX_CHECK_GLUT_PROGRAM],
                 [ax_cv_check_glut_libglut="${ax_try_lib}"; break])
done

AS_IF([test "X$ax_cv_check_glut_libglut" = Xno -a "X$no_x" = Xyes],
[LIBS='-framework GLUT'
AC_LINK_IFELSE([AX_CHECK_GLUT_PROGRAM],
               [ax_cv_check_glut_libglut="$LIBS"])])

CPPFLAGS="${ax_save_CPPFLAGS}"
LIBS="${ax_save_LIBS}"
AC_LANG_POP(C)])

AS_IF([test "X$ax_cv_check_glut_libglut" = Xno],
      [no_glut="yes"; GLUT_CFLAGS=""; GLUT_LIBS=""],
      [GLUT_LIBS="${ax_cv_check_glut_libglut} ${GLUT_LIBS}"])

AC_SUBST([GLUT_CFLAGS])
AC_SUBST([GLUT_LIBS])
])dnl
