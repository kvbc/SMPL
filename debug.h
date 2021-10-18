#ifndef SMPL_DEBUG_H
#define SMPL_DEBUG_H

#include "smpl.h"
#include <stdarg.h>


#ifdef SMPL_DEBUG
	#define spDO_print spD_print

	#define spDO_dectabs(S) ((S)->tabs -= 4)
	#define spDO_inctabs(S) ((S)->tabs += 4)

	#define spDO_incprint(S, ln, fmt, ...)				\
	(													\
		spDO_inctabs((S)),								\
		spDO_print((S), (ln), (fmt), ##__VA_ARGS__)		\
	)

	#define spDO_printinc(S, ln, fmt, ...)				\
	(													\
		spDO_print((S), (ln), (fmt), ##__VA_ARGS__),	\
		spDO_inctabs((S))								\
	)

	#define spDO_decprint(S, ln, fmt, ...)				\
	(													\
		spDO_dectabs((S)),								\
		spDO_print((S), (ln), (fmt), ##__VA_ARGS__)		\
	)

	#define spDO_printdec(S, ln, fmt, ...)				\
	(													\
		spDO_print((S), (ln), (fmt), ##__VA_ARGS__),	\
		spDO_dectabs((S))								\
	)


	#define spDO_asttopr	spD_asttopr
	#define spDO_astt2str	spD_astt2str

	#define spDO_printf printf
	#define spDO_puts	puts

	/*
	*
	* Parser
	*
	*/

	#define spPDO_print(S, fmt, ...)			spDO_print   ((S), (S)->ln - 1, (fmt), ##__VA_ARGS__)
	#define spPDO_incprint(S, fmt, ...)			spDO_incprint((S), (S)->ln - 1, (fmt), ##__VA_ARGS__)
	#define spPDO_printinc(S, fmt, ...)			spDO_printinc((S), (S)->ln - 1, (fmt), ##__VA_ARGS__)
	#define spPDO_decprint(S, fmt, ...)			spDO_decprint((S), (S)->ln - 1, (fmt), ##__VA_ARGS__)
	#define spPDO_printdec(S, fmt, ...)			spDO_printdec((S), (S)->ln - 1, (fmt), ##__VA_ARGS__)

	/*
	*
	* Compiler
	*
	*/

	#define spCDO_print(S, ast, fmt, ...)		spDO_print   ((S), (ast)->ln, (fmt), ##__VA_ARGS__)
	#define spCDO_incprint(S, ast, fmt, ...)	spDO_incprint((S), (ast)->ln, (fmt), ##__VA_ARGS__)
	#define spCDO_printinc(S, ast, fmt, ...)	spDO_printinc((S), (ast)->ln, (fmt), ##__VA_ARGS__)
	#define spCDO_decprint(S, ast, fmt, ...)	spDO_decprint((S), (ast)->ln, (fmt), ##__VA_ARGS__)
	#define spCDO_printdec(S, ast, fmt, ...)	spDO_printdec((S), (ast)->ln, (fmt), ##__VA_ARGS__)

	/*
	*
	* Virtual Machine
	*
	*/

	#define spVDO_print(S, pc, fmt, ...)		spDO_print   ((S), pc, (fmt), ##__VA_ARGS__)
	#define spVDO_incprint(S, pc, fmt, ...)		spDO_incprint((S), pc, (fmt), ##__VA_ARGS__)
	#define spVDO_printinc(S, pc, fmt, ...)		spDO_printinc((S), pc, (fmt), ##__VA_ARGS__)
	#define spVDO_decprint(S, pc, fmt, ...)		spDO_decprint((S), pc, (fmt), ##__VA_ARGS__)
	#define spVDO_printdec(S, pc, fmt, ...)		spDO_printdec((S), pc, (fmt), ##__VA_ARGS__)

#else
	#define spDO_print

	#define spDO_incprint
	#define spDO_printinc
	#define spDO_decprint
	#define spDO_printdec

	#define spDO_dectabs
	#define spDO_inctabs

	#define spDO_asttopr
	#define spDO_astt2str

	#define spDO_printf
	#define spDO_puts

	/*
	*
	* Parser
	*
	*/

	#define spPDO_print(S, fmt, ...)
	#define spPDO_incprint(S, fmt, ...)
	#define spPDO_printinc(S, fmt, ...)
	#define spPDO_decprint(S, fmt, ...)
	#define spPDO_printdec(S, fmt, ...)

	/*
	*
	* Compiler
	*
	*/

	#define spCDO_print(S, ast, fmt, ...)
	#define spCDO_incprint(S, ast, fmt, ...)
	#define spCDO_printinc(S, ast, fmt, ...)
	#define spCDO_decprint(S, ast, fmt, ...)
	#define spCDO_printdec(S, ast, fmt, ...)

	/*
	*
	* Virtual Machine
	*
	*/

	#define spVDO_print(S, pc, fmt, ...)
	#define spVDO_incprint(S, pc, fmt, ...)
	#define spVDO_printinc(S, pc, fmt, ...)
	#define spVDO_decprint(S, pc, fmt, ...)
	#define spVDO_printdec(S, pc, fmt, ...)
#endif


#endif // SMPL_DEBUG_H