AC_DEFUN([ATHEME_COND_CONTRIB_SUBMODULE_ENABLE], [

	CONTRIB_COND_D="contrib"
	AC_SUBST([CONTRIB_COND_D])
])

AC_DEFUN([ATHEME_COND_ECDSA_TOOLS_ENABLE], [

	ECDSA_TOOLS_COND_D="ecdsadecode ecdsakeygen ecdsasign"
	AC_SUBST([ECDSA_TOOLS_COND_D])
])

AC_DEFUN([ATHEME_COND_LIBMOWGLI_SUBMODULE_ENABLE], [

	LIBMOWGLI_COND_D="libmowgli-2"
	AC_SUBST([LIBMOWGLI_COND_D])
])

AC_DEFUN([ATHEME_COND_LEGACY_PWCRYPTO_ENABLE], [

	LEGACY_PWCRYPTO_COND_C="crypt3-des.c crypt3-md5.c ircservices.c rawmd5.c rawsha1.c"
	AC_SUBST([LEGACY_PWCRYPTO_COND_C])
])

AC_DEFUN([ATHEME_COND_PERL_ENABLE], [

	PERL_COND_D="perl"
	AC_SUBST([PERL_COND_D])
])

AC_DEFUN([ATHEME_COND_QRCODE_ENABLE], [

	QRCODE_COND_C="qrcode.c"
	AC_SUBST([QRCODE_COND_C])
])