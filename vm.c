/*
*
* vm.c
* (Virtual Machine)
* Instruction execution - the heart of an interpreter.
*
*/


/* SMPL headers */
#include "state.h"
#include "parser.h"
#include "opcodes.h"
#include "smpl.h"
#include "limits.h"
#include "mem.h"
#include "debug.h"
#include "api.h"


static sp_Label *getlbl (sp_State *S, uint lbl)
{
	for(uint j = 0; j < S->lbltop; ++j)
	{
		if(S->lbls[j].id == lbl)
		{
			return &S->lbls[j];
		}
	}
	return NULL;
}


#define insat(S,i)		((S)->inss[(i)])			/* get the instruction at index 'i' */
#define ins(S)			insat((S), (*pc)++)			/* get the current instruction and advance the program counter */

#define argat(S,i)		((S)->opargs[(i)])			/* get the instruction argument at index 'i' */
#define getarg(S)		argat((S), GET_ARG(i))		/* get an argument attached to the current instruction */

//#define vmdispatch(op)	switch((op))
#define vmcase(op)		case (op):
#define vmbreak			break
#define vmnext			continue

#define savepc(S,ofs)	(sp_push_number((S), *pc + (ofs)))					/* save the current program counter */
#define restorepc(S)	((*pc = obj_num(sp_top((S), 0))), (sp_pop((S), 1)))	/* restore the last program counter */

#define VMSTATE_ARGS 	sp_State *S, uint *pc, sp_Instruction end, sp_ObjectType m, sp_Instruction i
#define VMSTATE 		S, pc, end, m, i

static sp_Dim *get_dim (VMSTATE_ARGS)
{
	sp_Object *arg = getarg(S);
	sp_String *id = obj_str(arg);
	sp_Object *o = sp_Sget_odglobal(S, id);
	sp_Dim *dim = obj_dim(o);

	return dim;
}

static uint32_t get_dim_index (VMSTATE_ARGS, sp_Dim *dim)
{
	uint32_t ai = 0; /* absolute index */
	uint32_t offset = 1;

	uint32_t dims = ins(S);

	// 2, 2, 2

	// 1, 1, 1
	// 1, 1, 2
	// 1, 2, 1
	// 1, 2, 2
	// 2, 1, 1
	// 2, 1, 2
	// 2, 2, 1
	// 2, 2, 2
	// ai = 7

	/*
	ai += 1
	ai += 2
	ai += 4
	*/

	for(uint32_t i = 0; i < dims; i++)
	{
		uint32_t num = sp_checknum(S);
		ai += (num - 1) * offset;
		offset *= dim->lens[i];
	}

	return ai;
}


/*
** Evaluate next instructions until the instruction is equal to 'end'
*/
static void exec (sp_State *S, uint *pc, sp_Instruction end, sp_ObjectType m)
{
	sp_Instruction i;
	while(*pc < S->instop && (i = ins(S)) != end)
	{
		sp_OpCode opc = GET_OPCODE(i);
		spVDO_printinc(S, *pc - 1, "> %s ", spD_op2str(opc));

		#define vmdispatch(op) switch((op))
		vmdispatch(opc)
		{
			vmcase(OP_DUP)
			{
				sp_push(S, spO_new_copy(S, sp_top(S, 0)));
				vmbreak;
			}
			vmcase(OP_POP)
			{
				sp_pop(S, 1);
				vmbreak;
			}
			vmcase(OP_PRINTNL)
			{
				sp_io_printstr(S, "\n");
				vmbreak;
			}
			vmcase(OP_INPUT)
			{
				sp_io_getstr(S);
				sp_push_String(S, S->s);
				vmbreak;
			}
			vmcase(OP_PRINT)
			{
				sp_print(S);
				vmbreak;
			}
			vmcase(OP_LOADS)
			{
				sp_Object *arg = getarg(S);
				sp_String *s = obj_str(arg);
				spDO_printf("\"%s\"", s->src);
				sp_push_String(S, s);
				vmbreak;
			}
			vmcase(OP_LOADN)
			{
				sp_Object *arg = getarg(S);
				sp_Number n = obj_num(arg);
				spDO_printf("%lf", n);
				sp_push_number(S, n);
				vmbreak;
			}
			vmcase(OP_SETVAR)
			{
				sp_Object *arg = getarg(S);
				sp_String *id = obj_str(arg);
				spDO_printf("\"%s\"", id->src);
				sp_Sset_global(S, id);
				vmbreak;
			}
			vmcase(OP_DIM)
			{
				uint32_t argi = GET_ARG(i);
				sp_Object *arg = getarg(S);
				sp_Object *o = argat(S, argi + 1);
				sp_Dim *dim = obj_dim(o);

				uint32_t dims = ins(S);

				uint32_t len = 1; /* intial length - to be determined */
				uint32_t *lens = malloc(dims * sizeof(*lens)); /* length of individual "dimensions" */

				for(uint32_t di = 0; di < dims; di++)
				{
					uint32_t num = sp_checknum(S);
					len *= num;
					lens[di] = num;
				}

				sp_Object **v = calloc(len, sizeof(*v)); /* calloc values */
				obj_setdim(o, spA_new_set(S, v, len)); /* allocate new DIM and assign it's value to object */
				obj_settype(o, OT_DIM);

				dim = obj_dim(o);
				dim->lens = lens; /* TODO: put into "spA_new_set" */

				vmbreak;
			}
			vmcase(OP_GETNAME)
			{

				sp_Object *arg = getarg(S);
				sp_String *id = obj_str(arg);
				spDO_printf("\"%s\"", id->src);
				sp_Smget_global(S, m, id);
			#ifdef SMPL_DEBUG
				/*
				 * TODO: Remaster
				 */
				spDO_printf(" (");
				sp_push(S, spO_new_copy(S, sp_top(S, 0)));
				sp_print(S);
				spDO_printf(")");
			#endif
				vmbreak;
			}
			vmcase(OP_SETDIM)
			{
				sp_Dim *dim = get_dim(VMSTATE);
				uint32_t ai = get_dim_index(VMSTATE, dim);

				// printf("1\n");
				sp_Object *obj = dim->src[ai];
				// printf("2\n");
				if(obj == NULL)
				{
					dim->src[ai] = spO_new(S);
					obj = dim->src[ai];
				}
				// printf("3\n");
				// printf(">%p\n", obj);
				spO_copy(S, obj, sp_top(S, 0));
				// printf("4\n");
				sp_pop(S, 1);

				vmbreak;
			}
			vmcase(OP_GETDIM)
			{
				sp_Dim *dim = get_dim(VMSTATE);
				uint32_t ai = get_dim_index(VMSTATE, dim);

				sp_Object *obj = dim->src[ai];
				if(obj != NULL)
					sp_push(S, spO_new_copy(S, obj));

				vmbreak;
			}
			vmcase(OP_SAVE)
			{
				savepc(S, 3);
				vmbreak;
			}
			vmcase(OP_RESTORE)
			{
				restorepc(S);
				vmbreak;
			}
			vmcase(OP_FN)
			{
				sp_Object *arg = getarg(S);
				sp_String *fid = obj_str(arg);
				sp_Object *f = sp_Sget_ofglobal(S, fid);
				sp_ObjectType objt = OT_NUM | OT_STR;

				sp_Object *ov; /* old value */
				sp_Object *v = sp_Smget_oglobal(S, objt, obj_fsarg(f)); /* variable that argument possibly replaces */
				
				/* argument replaces an variable, save it */
				if(v != NULL)
				{
					ov = spO_new_copy(S, v);
				}
				
				sp_Smset_oglobal(S, objt, obj_fsarg(f), sp_top(S, 0));
				sp_pop(S, 1);

				spDO_printf("\"%s\"\n", fid->src);

				uint opc = *pc; /* save current pc */
				*pc = obj_fi(f); /* jump to function's source */
				exec(S, pc, OP_FNEND, objt); /* execute to the end of function */
				*pc = opc; /* restore the saved pc */

				/* restore the old variable */
				if(v != NULL)
				{
					sp_Smset_oglobal(S, objt, obj_fsarg(f), ov);
					spO_free(ov);
				}

				spDO_dectabs(S);
				vmnext;
			}
			vmcase(OP_DEF)
			{
				uint argi = GET_ARG(i);
				uint fsrc = *pc + 2;					 /* function src - skip the jump to the end */
				sp_String *fid = obj_str(argat(S,argi)); /* function id (first oparg) */
				sp_Object *farg = argat(S, argi + 1);    /* function arg (second oparg) */
				spDO_printf("\"%s\"", fid->src);
				sp_Sset_ofglobal(S, fid, fsrc, farg);
				vmbreak;
			}
			vmcase(OP_CALL)
			{
				sp_Object *o = getarg(S);
				spDO_printf("\"%s\"", obj_str(o)->src);
				sp_Sget_cfglobal(S, obj_str(o));
				sp_call(S, ins(S));
				vmbreak;
			}
			vmcase(OP_JUMP)
			{
				uint npc = ins(S);
				spDO_printf("%u", npc);
				*pc = npc;
				vmbreak;
			}
			vmcase(OP_JLBL)
			{
				uint ln = ins(S);
				uint lblid = ins(S);
				// printf("label id: %d\n", lblid);
				sp_Label *lbl = getlbl(S, lblid);
				*pc = lbl->i;
				spDO_printf("%u (LBL %u)", *pc, lblid);
				vmbreak;
			}
			vmcase(OP_JTRUE)
			{
				uint npc = S->inss[*pc];
				spDO_printf("%u", npc);
				if(sp_checknum(S))
					*pc = npc;
				else ++*pc;
				vmbreak;
			}
			vmcase(OP_JFALSE)
			{
				uint npc = S->inss[*pc];
				spDO_printf("%u", npc);
				if(!sp_checknum(S))
					*pc = npc;
				else ++*pc;
				vmbreak;
			}
			vmcase(OP_NEG)
			{
				spDO_printf("-");
				sp_unary_arith(S, OPR_NEG);
				vmbreak;
			}
			vmcase(OP_NOT)
			{
				spDO_printf("!");
				sp_unary_arith(S, OPR_NOT);
				vmbreak;
			}
			vmcase(OP_OR)
			{
				spDO_printf("||");
				sp_binary_arith(S, OPR_OR);
				vmbreak;
			}
			vmcase(OP_AND)
			{
				spDO_printf("&&");
				sp_binary_arith(S, OPR_AND);
				vmbreak;
			}
			vmcase(OP_LT)
			{
				spDO_printf("<");
				sp_binary_arith(S, OPR_LT);
				vmbreak;
			}
			vmcase(OP_LE)
			{
				spDO_printf("<=");
				sp_binary_arith(S, OPR_LE);
				vmbreak;
			}
			vmcase(OP_GT)
			{
				spDO_printf(">");
				sp_binary_arith(S, OPR_GT);
				vmbreak;
			}
			vmcase(OP_GE)
			{
				spDO_printf(">=");
				sp_binary_arith(S, OPR_GE);
				vmbreak;
			}
			vmcase(OP_NE)
			{
				spDO_printf("!=");
				sp_binary_arith(S, OPR_NE);
				vmbreak;
			}
			vmcase(OP_EQ)
			{
				spDO_printf("==");
				sp_binary_arith(S, OPR_EQ);
				vmbreak;
			}
			vmcase(OP_PLUS)
			{
				spDO_printf("+");
				sp_binary_arith(S, OPR_PLUS);
				vmbreak;
			}
			vmcase(OP_MINUS)
			{
				spDO_printf("-");
				sp_binary_arith(S, OPR_MINUS);
				vmbreak;
			}
			vmcase(OP_MUL)
			{
				spDO_printf("*");
				sp_binary_arith(S, OPR_MUL);
				vmbreak;
			}
			vmcase(OP_DIV)
			{
				spDO_printf("/");
				sp_binary_arith(S, OPR_DIV);
				vmbreak;
			}
			vmcase(OP_MOD)
			{
				spDO_printf("%");
				sp_binary_arith(S, OPR_MOD);
				vmbreak;
			}
		}

		spDO_dectabs(S);
		spDO_puts("");
	}
}


/*
** Main interpreter loop
*/
void spV_execute (sp_State *S)
{
	spDO_printf("========================================================================================\n"
	            "| EXECUTION\n"
		        "========================================================================================\n");

#ifdef SMPL_DEBUG
    if(S->instop)
	{
		puts("[PC]-----------------------------------------------");
		for(int i = 0; i < S->instop; ++i)
		{
			sp_OpCode op = GET_OPCODE(S->inss[i]);
			printf("[%2u] %-7s [%2u]\n", i, spD_op2str(op), op);
			switch(op)
			{
				case OP_JLBL:
					spDO_incprint(S, ++i, "> line number\n");
					spDO_printdec(S, ++i, "> label\n");
					continue;
				case OP_JTRUE:
				case OP_JFALSE:
					spDO_incprint(S, ++i, "> condition\n");
					spDO_dectabs(S);
					continue;
			}
		}
		puts("\n[PC]-----------------------------------------------");
	}
#endif

	uint pc = 0;
	exec(S, &pc, OP_END, OT_ANY); /* execute the whole program */
}