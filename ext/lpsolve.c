/*  Copyright (C) 2007, 2010 Rocky Bernstein <rockyb@rubyforge.org>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <ruby.h>
#include <stdio.h>
#include <lpsolve/lp_lib.h>
#include <lpsolve/lp_report.h>

/** \file lpsolve.c
 *
 *  \brief Provides Ruby access to lpsolve.

    Ruby LPSolve class is created.

    In the documentation below, change "lpsolve_" to LPSolve:: to 
    get the corresponding Ruby method name. For example lpsolve_get_mat()
    is visible in Ruby as LPSolve::get_mat().
 */

#define DEBUG_GC 0

#define SOLVE_NOT_CALLED -10

/* Not part of API (yet). Return a column number for a given column name. 
   -1 is returned if the column was not found.
*/
static int 
get_col_num(lprec *lp, const char *psz_col_name) 
{
  if (lp->names_used && lp->use_col_names) {
    unsigned int i;
    for (i=0; i<= lp->columns; i++) {
      if ((lp->col_name[i] != NULL) && (lp->col_name[i]->name != NULL)) {
#ifdef Paranoia
      if(lp->col_name[i]->index != i) 
	report(lp, SEVERE, 
	       "%s: Inconsistent column ordinal %d vs %d\n",
	       __FUNCTION__, i, lp->col_name[i]->index);
#endif
      if (0 == strcmp(psz_col_name, lp->col_name[i]->name))
	return i;
      }
    }
  }
  /* Not found. */
  return -1;
}

/** \def INIT_LP 

Boilerplate beginning of all functions: declares a pointer to the lp
structure, gets the internal object from Ruby (basically "self") and if
that's nil, fail immediately.
*/
#define INIT_LP					\
lprec *lp;					\
Data_Get_Struct(self, lprec, lp);		\
if (NULL == lp) return Qnil;			\


#define RETURN_BOOL(x) \
  return (x) ? Qtrue : Qfalse

/** \def LPSOLVE_0_IN_BOOL_OUT

   A macro used in defining a Ruby method which takes no arguments
   other than self and calls the corresponding C function returns the
   boolean static back to Ruby.  That is, if lp is not \a NULL we
   return Qtrue. If lp is \a NULL, we return \a Qfalse.
*/
#define LPSOLVE_0_IN_BOOL_OUT(fn)			\
  static VALUE					        \
  lpsolve_ ## fn(VALUE self)				\
  {							\
    INIT_LP;						\
    RETURN_BOOL(fn(lp));				\
  }

/** \def LPSOLVE_0_IN_INT_OUT 

   A macro used in defining a Ruby method which takes no arguments
   other than self and calls the corresponding C function returns the
   integer return value back to Ruby.  That is, if \a lp is not \a
   NULL. If it's \a NULL, we return \a nil.
*/
#define LPSOLVE_0_IN_INT_OUT(fn)				\
  static VALUE							\
  lpsolve_ ## fn(VALUE self)					\
  {								\
    INIT_LP;							\
    return (NULL != lp) ? INT2FIX(fn(lp)) : Qnil;		\
  }

/** \def LPSOLVE_0_IN_NUM_OUT 

   A macro used in defining a Ruby method which takes no arguments
   other than self and calls the corresponding C function returns the
   number return value back to Ruby.  That is, if \a lp is not \a
   NULL. If it's \a NULL, we return \a nil.
*/
#define LPSOLVE_0_IN_NUM_OUT(fn)				\
  static VALUE							\
  lpsolve_ ## fn(VALUE self)					\
  {								\
    INIT_LP;							\
    return rb_float_new(fn(lp));				\
  }

/** \def LPSOLVE_0_IN_STATUS_OUT

   A macro used in defining a Ruby method which takes no arguments
   other than self and calls the corresponding C function returns the
   boolean statuc back to Ruby.  That is, if lp is not \a NULL we
   return Qtrue. If lp is \a NULL, we return \a Qfalse.
*/
#define LPSOLVE_0_IN_STATUS_OUT(fn)			\
  static VALUE					        \
  lpsolve_ ## fn(VALUE self)				\
  {							\
    INIT_LP;						\
    fn(lp);						\
    return Qtrue;					\
  }

/** \def LPSOLVE_0_IN_TIME_OUT 

   A macro used in defining a Ruby method which takes no arguments
   other than self returns a floating point number which is the 
   difference between to floating-point fields - time values.
*/
#define LPSOLVE_0_IN_TIME_OUT(fn, field1, field2)			\
  static VALUE								\
  lpsolve_ ## fn(VALUE self)						\
  {									\
    INIT_LP;								\
    int status = NUM2INT(rb_ivar_get(self, rb_intern("@status")));	\
    if (SOLVE_NOT_CALLED == status)					\
      return rb_float_new(0);						\
    return rb_float_new(lp->field2 - lp->field1);			\
  }

/** \def LPSOLVE_1_BOOL_IN_BOOL_OUT

   A macro used in defining a Ruby method which takes one boolean
   argument other than self and calls the corresponding C function
   returns the boolean statuc back to Ruby.  That is, if lp is not \a
   NULL we return Qtrue. If lp is \a NULL, we return \a Qnil.
*/
#define LPSOLVE_1_BOOL_IN_BOOL_OUT(fn)				\
  static VALUE							\
  lpsolve_ ## fn(VALUE self, VALUE param1)			\
  {								\
    INIT_LP;							\
    if (param1 != Qtrue && param1 != Qfalse) {			\
      report(lp, IMPORTANT,					\
	     "%s: Parameter is not a boolean.\n",		\
	     __FUNCTION__);					\
      return Qnil;						\
    } else {							\
      unsigned char turnon = param1 == Qtrue ? 1 : 0;		\
      RETURN_BOOL(fn(lp, turnon));				\
    }								\
  }

/** \def LPSOLVE_1_IN_BOOL_OUT

   A macro used in defining a Ruby method which takes one argument
   other than self and calls the corresponding C function returns the
   boolean statuc back to Ruby.  That is, if lp is not \a NULL we
   return Qtrue. If lp is \a NULL, we return \a Qnil

   \a type is a Ruby typename like \a T_FIXNUM, \a typename is a type
   name string to use in an error message, e.g "an integer"; \a
   convert a Ruby conversion macro like \a FIX2INT.
*/
#define LPSOLVE_1_IN_BOOL_OUT(fn, type, typename, convert)	\
  static VALUE							\
  lpsolve_ ## fn(VALUE self, VALUE param1)			\
  {								\
    INIT_LP;							\
    if (TYPE(param1) != type) {					\
      report(lp, IMPORTANT,					\
	     "%s: Parameter is not %s.\n",			\
	     __FUNCTION__, typename);				\
      return Qnil;						\
    }								\
    RETURN_BOOL(fn(lp, convert(param1)));			\
  }

/** \def LPSOLVE_1_IN_NUM_OUT 

   A macro used in defining a Ruby method which takes one argument
   along with self and calls the corresponding C function returns the
   integer return value back to Ruby.  That is, if \a lp is not \a
   NULL. If it's \a NULL, we return \a nil.

   \a type is a Ruby typename like \a T_FIXNUM, \a typename is a type
   name string to use in an error message, e.g "an integer"; \a
   convert a Ruby conversion macro like \a FIX2INT.
   \a fn is the lpsolve function to call. 

*/
#define LPSOLVE_1_IN_NUM_OUT(fn, type, typename, convert)	\
  static VALUE							\
  lpsolve_ ## fn (VALUE self, VALUE param1)			\
  {								\
    INIT_LP							\
    if (TYPE(param1) != type) {					\
      report(lp, IMPORTANT,					\
	     "%s: Parameter is not %s.\n",			\
	     __FUNCTION__, typename);				\
      return Qnil;						\
    }								\
    return rb_float_new(fn(lp, convert(param1)));		\
  }

/** \def LPSOLVE_1_IN_STR_OUT 

   A macro used in defining a Ruby method which takes one argument
   along with self and calls the corresponding C function returns a
   string  back to Ruby.  That is, if \a lp is not \a
   NULL. If it's \a NULL, we return \a nil.

   \a arg1 is passed to the \a lpsolve routine. It should be 
   a function of \a param1 which is the parameter of the wrapper
   function. For example \a arg1 could be \a FIX2INT(param1).

*/
#define LPSOLVE_1_IN_STR_OUT(fn, type, typename, convert)	\
  static VALUE							\
  lpsolve_ ## fn (VALUE self, VALUE param1)			\
  {								\
    INIT_LP							\
    if (TYPE(param1) != type) {					\
      report(lp, IMPORTANT,					\
	     "%s: Parameter is not %s.\n",			\
	     __FUNCTION__, typename);				\
      return Qnil;						\
    }								\
    return rb_str_new2(fn(lp, convert(param1)));		\
  }

/** \def LPSOLVE_1_IN_BOOL_OUT 

   A macro used in defining a Ruby method which takes one argument
   along with self and calls the corresponding C function returns the
   integer return value back to Ruby.  That is, if \a lp is not \a
   NULL. If it's \a NULL, we return \a nil.
*/
#define LPSOLVE_1_STR_IN_BOOL_OUT(fn)				\
  static VALUE							\
  lpsolve_ ## fn (VALUE self, VALUE str)			\
  {								\
    INIT_LP;							\
    if (TYPE(str) != T_STRING) {				\
      report(lp, IMPORTANT,					\
	     "%s: Parameter 1 is not a string\n",		\
	     __FUNCTION__);					\
      return Qfalse;						\
    }								\
    RETURN_BOOL(fn(lp, RSTRING_PTR(str)));			\
  }

/** \def LPSOLVE_1_IN_STATUS_OUT 

   A macro used in defining a Ruby method which takes one argument
   along with self and calls the corresponding C function returns the
   integer return value back to Ruby.  That is, if \a lp is not \a
   NULL. If it's \a NULL, we return \a nil.

   \a arg1 is passed to the \a lpsolve routine. It should be 
   a function of \a param1 which is the parameter of the wrapper
   function. For example \a arg1 could be \a FIX2INT(param1).

*/
#define LPSOLVE_1_IN_STATUS_OUT(fn, arg1)			\
  static VALUE							\
  lpsolve_ ## fn (VALUE self, VALUE param1)			\
  {								\
    INIT_LP							\
    fn(lp, arg1);						\
    return Qtrue;						\
  }

#define LPSOLVE_SET_VARTYPE(fn)						\
static VALUE								\
 lpsolve_ ## fn (VALUE self, VALUE column_num, 	VALUE new_bool)		\
{									\
  INIT_LP;								\
  if (new_bool != Qtrue && new_bool != Qfalse && new_bool != Qnil) {	\
    report(lp, IMPORTANT,						\
	   "%s: Parameter is not a boolean or nil.\n",			\
	   __FUNCTION__);						\
    return Qfalse;							\
  } else {								\
    RETURN_BOOL(fn(lp, FIX2INT(column_num), Qtrue == new_bool));	\
  }									\
}


static VALUE
lpsolve_set_binary (VALUE self, VALUE column_num, 	VALUE new_bool)
{
  INIT_LP;
  if (new_bool != Qtrue && new_bool != Qfalse && new_bool != Qnil) {
    report(lp, IMPORTANT,
	   "%s: Parameter is not a boolean or nil.\n",
	   __FUNCTION__);
    return Qfalse;
  } else {
    RETURN_BOOL(set_binary(lp, FIX2INT(column_num), Qtrue == new_bool));
  }
}


/** Holder for LPSolve class object. A singleton value. */
VALUE rb_cLPSolve;

static void lpsolve_free(void *lp);

/** 
 A wrapper for add_constraintex.

 Adds a constraint. These routines will perform much better when
 lpsolve_set_add_rowmode() is called before adding constraints.

 @param self self
 @param name The name of the constraint or \a nil if no name.

 @param row_coeffs A list of tuples. The first entry of the tuple is
 the column number which should be in the range 0..columns-1; the
 second entry in the tuple should be the coefficient value.

 @param constr_type The constraint type. Should be one of 
 \a LPSolve::LE, \a LPSolve::EQ, \a LPSolve::GE.

 @param rh The right-hand-side constant, a number.

 @return number of rows, (the row number of the constraint added) if
 successful or \a nil on error.

*/
static VALUE 
lpsolve_add_constraintex(VALUE self, VALUE name, VALUE row_coeffs, 
			 VALUE constr_type, VALUE rh) 
{
  int i_constr_type = FIX2INT(constr_type);
  REAL r_rh = NUM2DBL(rh);
  REAL *row = NULL;
  int *colno = NULL;
  long int i, count;
  VALUE ret = Qnil;
  MYBOOL b_ret;
  VALUE *p_row_coeff = NULL;

  INIT_LP;

  if (TYPE(name) != T_STRING && name != Qnil) {
    report(lp, IMPORTANT, 
	   "%s: constraint name, parameter 1, should be nil or a string.\n", 
	   __FUNCTION__);
    return Qnil;
  }

  if (TYPE(row_coeffs) != T_ARRAY) {
    report(lp, IMPORTANT, 
	   "%s: row coefficients, parameter 2 is not an array.\n",
	   __FUNCTION__);
    return Qnil;
  }

  if (TYPE(constr_type) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: constraint type, parameter 3, is not a number.\n", 
	   __FUNCTION__);
    return Qnil;
  }

  switch (i_constr_type) {
  case EQ:
  case GE:
  case LE: break;
  default:
    report(lp, IMPORTANT, 
	   "%s: constraint type, parameter 3, should be LE, EQ, or GE.\n", 
	   __FUNCTION__);
    return Qnil;
  }
  
  /***FIXME: combine common parts of this with add_constraintex ****/
  count   = RARRAY_LEN(row_coeffs);

  if (0 == count)  {
    report(lp, IMPORTANT, 
	   "%s: row coefficients array has to have at least one item.\n",
	   __FUNCTION__);
    return Qnil;
  }

  colno   = ALLOC_N(int, count);
  row     = ALLOC_N(REAL, count);

  p_row_coeff = RARRAY_PTR(row_coeffs);
  for (i = 0; i < count; i++) {
    int i_col;
    if (TYPE(*p_row_coeff) != T_ARRAY) {
      report(lp, IMPORTANT, 
	     "%s: row coeffient element %d is not an array.\n", 
	     __FUNCTION__, i);
      goto done;
    }
    if (RARRAY_LEN(*p_row_coeff) != 2) {
      report(lp, IMPORTANT, 
	     "%s: row coeffient element %d is not an array tuple.\n", 
	     __FUNCTION__, i);
      goto done;
    } else {
      VALUE *tuple = RARRAY_PTR(*p_row_coeff);
      if (TYPE(tuple[0]) != T_FIXNUM) {
	report(lp, IMPORTANT, 
	       "%s: Column number, first element, of row coefficients at " \
	       "tuple %d is not a integer.\n", __FUNCTION__, i);
	goto done;
      }
      switch (TYPE(tuple[1])) {
      case T_FIXNUM:
      case T_FLOAT: break;
      default:
	report(lp, IMPORTANT, 
	       "%s: Coefficient value, second element, of row coeffients at "
	       "tuple %d is not an integer.\n", __FUNCTION__, i);
	goto done;
      }

      i_col = FIX2INT(tuple[0]);
      if (i_col <= 0 || i_col > lp->columns) {
	report(lp, IMPORTANT, 
	       "%s: Column number, first element, of row coeffients at " \
	       "tuple %d, value %d, is not in the range 1..%d\n", 
	       __FUNCTION__, i, i_col, lp->columns);
	goto done;
      }
      colno[i] = i_col;
      row[i]   = NUM2DBL(tuple[1]);
    }
    p_row_coeff++;
  }

  b_ret = add_constraintex(lp, count, row, colno, i_constr_type, r_rh);
  if (b_ret) {
    ret = INT2FIX(lp->rows);
    if (name != Qnil) 
      set_row_name(lp, lp->rows, RSTRING_PTR(name));
  }

 done:
  free(row);
  free(colno);
  return ret;
}

/** 
 A wrapper for add_SOS.

 Adds an SOS constraint. A Special Ordered Set of Type n is a way of
 indicating that at most n of a set of variables may be nonzero.

 @param self self
 @param name The name of the SOS constraint.

 @param sos_type The type of the SOS constraint, 1 means "at most 1" 2
 means "at most 2".  Must be >= 1.
 @param priority Priority of the SOS constraint in the SOS set.
 @param sos_vars  A list of tuples of column numbers and weights

*/
static VALUE 
lpsolve_add_SOS(VALUE self, VALUE name, VALUE sos_type, VALUE priority, 
		VALUE sos_vars) 
{
  int i_sos_type = FIX2INT(sos_type);
  int i_priority = FIX2INT(priority);
  int *vars = NULL;
  double *weights = NULL;
  long int i, count;
  VALUE *p_sos_var;
  VALUE ret = Qnil;
  int i_ret = 0;

  INIT_LP;
  
  if (TYPE(sos_type) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: SOS type, parameter 2, is not a number.\n", __FUNCTION__);
    return Qnil;
  }
  
  if (TYPE(priority) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: priority, parameter 3, not a number.\n", __FUNCTION__);
    return Qnil;
  }
  if (TYPE(name) != T_STRING) {
    report(lp, IMPORTANT, "%s: name is not a string.\n", __FUNCTION__);
    return Qnil;
  }
  if (i_sos_type < 1) {
    report(lp, IMPORTANT, "%s: SOS type (%ld) is less than 1.\n",
	   __FUNCTION__, i_sos_type);
    return Qnil;
  }
  
  if (TYPE(sos_vars) != T_ARRAY) {
    report(lp, IMPORTANT, "%s: SOS vars is not an array.\n", __FUNCTION__);
    return Qnil;
  }

  count   = RARRAY_LEN(sos_vars);

  if (0 == count)  {
    report(lp, IMPORTANT, 
	   "%s: SOS vars array has to have at least one item.\n",
	   __FUNCTION__);
    return Qnil;
  }

  vars    = ALLOC_N(int, count);
  weights = ALLOC_N(double, count);

  p_sos_var = RARRAY_PTR(sos_vars);
  for (i = 0; i < count; i++) {
    if (TYPE(*p_sos_var) != T_ARRAY) {
      report(lp, IMPORTANT, 
	     "%s: SOS vars element %d is not an array.\n", __FUNCTION__, i);
      goto done;
    }
    if (RARRAY_LEN(*p_sos_var) != 2) {
      report(lp, IMPORTANT, 
	     "%s: SOS vars element %d is not an array tuple.\n", __FUNCTION__,
	     i);
      goto done;
    } else {
      VALUE *tuple = RARRAY_PTR(*p_sos_var);
      if (TYPE(tuple[0]) != T_FIXNUM) {
	report(lp, IMPORTANT, 
	       "%s: First element of SOS vars at tuple %d " \
	       "is not a integer.\n",  __FUNCTION__, i);
	goto done;
      }
      if (TYPE(tuple[1]) != T_FIXNUM) {
	report(lp, IMPORTANT, 
	       "%s: Second element of SOS vars at tuple %d " \
	       "is not an integer.\n", __FUNCTION__, i);
	goto done;
      }
      vars[i] = FIX2INT(tuple[0]);
      weights[i] = FIX2INT(tuple[1]);
    }
    p_sos_var++;
  }
  i_ret = add_SOS(lp, RSTRING_PTR(name), i_sos_type, i_priority, 
		  count, vars, weights);
  if (i_ret != 0)
    ret = INT2FIX(i_ret);

 done:
  free(vars);
  free(weights);
  return ret;
}

/** Allocate a Ruby container for a lprec * pointer. Ruby may call
    this routine directly under various circumstances such as when
    doing data marshalling. However usually the most common case is
    when new() is called.  Here, after the call to lpsolve_alloc(),
    Ruby will call lpsolve_initialize(). 
*/
static VALUE 
lpsolve_alloc(VALUE klass) 
{
  VALUE obj;
  obj = Data_Wrap_Struct(klass, 0, lpsolve_free, NULL);
  return obj;
}

/** A wrapper for default_basis().

    Sets the starting base to an all slack basis (the default simplex
    starting basis).
    
    @param self self
    @return \a true unless we have an error.
*/
static VALUE lpsolve_default_basis(VALUE self);
LPSOLVE_0_IN_STATUS_OUT(default_basis)

/** A wrapper for del_column().

    @param self self
    @param column_num column number

    @return \a true if the operation was successful. false indicates an
    error.  An error occurs when column is not between 1 and the number
    of columns in the lp.  Note that row entry mode must be off, else
    this function also fails.

    @see lpsolve_set_add_rowmode()
*/
static VALUE lpsolve_del_column(VALUE self, VALUE column_num);
LPSOLVE_1_IN_BOOL_OUT(del_column, T_FIXNUM, "an integer", FIX2INT);

/** A wrapper for del_constraint()

  del_constraint returns true if the operation was successful. 

  @param self self
  @param row_num row number

  @return a \a false value indicates an error.  An error occurs when
  row_num is not between 1 and the number of rows self.  Note that row
  entry mode must be off, else this function also fails. @see
  lpsolve_set_add_rowmode().
*/
static VALUE lpsolve_del_constraint(VALUE self, VALUE row_num);
LPSOLVE_1_IN_BOOL_OUT(del_constraint, T_FIXNUM, "an integer", FIX2INT);


static void
lpsolve_free(void *lp) 
{
#if DEBUG_GC
  printf("lpsolve_free called\n");
#endif
  if (NULL != lp) delete_lp((lprec *)lp);
}

/** A wrapper for get_bb_depthlimit(lprec *lp);

   get_bb_depthlimit returns the maximum branch-and-bound depth. 

   Parameters

   lp

   Pointer to previously created lp model. See return value of make_lp, copy_lp, read_lp, read_LP, read_mps, read_freemps, read_MPS, read_freeMPS, read_XLI

   Remarks
   
   The get_bb_depthlimit function returns the maximum branch-and-bound
   depth.  This is only useful if there are integer, semi-continious
   or SOS variables in the model so that the branch-and-bound
   algorithm must be used to solve them. The branch-and-bound
   algorithm will not go deeper than this level. When 0 then there is
   no limit to the depth. Limiting the depth will speed up solving
   time, but there is a chance that the found solution is not the most
   optimal one. Be aware of this. It can also result in not finding a
   solution at all. A positive value means that the depth is
   absolute. NOTE: in the standard lpsolve an absolute depth is
   *added* to the number of SOS vars and semi-continuous vars. In a
   hacked version of lpsolve, absolute is, well, absolute.

   A negative value means a relative B&B depth limit. The
   "order" of a MIP problem is defined to be 2x the number of binary
   variables plus the number of SC and SOS variables. A relative value
   of -x results in a maximum depth of x times the order of the MIP
   problem.  The default is -50.  The get_bb_rule function returns the
   branch-and-bound rule for choosing which non-integer variable is to
   be selected. This rule can influence solving times
   considerably. Depending on the model one rule can be best and for
   another model another rule.  The default is NODE_PSEUDONONINTSELECT
   + NODE_GREEDYMODE + NODE_DYNAMICMODE + NODE_RCOSTFIXING (17445).

    @param self self
    @return  Returns the branch-and-bound rule.
*/
static VALUE lpsolve_get_bb_depthlimit(VALUE self);
LPSOLVE_0_IN_NUM_OUT(get_bb_depthlimit);

/** A wrapper for get_bb_rule().

    The get_bb_rule function returns the branch-and-bound rule for
    choosing which non-integer variable is to be selected. This rule
    can influence solving times considerably. Depending on the model
    one rule can be best and for another model another rule.  The
    default is NODE_PSEUDONONINTSELECT + NODE_GREEDYMODE +
    NODE_DYNAMICMODE + NODE_RCOSTFIXING (17445).

    @param self self
    @return  Returns the branch-and-bound rule.

*/
static VALUE lpsolve_get_bb_rule(VALUE self);
LPSOLVE_0_IN_NUM_OUT(get_bb_rule);

/** A wrapper for get_col_name().

    get_col_name() returns the name of the specified column.  The
    difference between get_col_name and get_origcol_name is only
    visible when a presolve (set_presolve) was done. Presolve can
    result in deletion of columns in the model. In get_col_name(),
    column specifies the column number after presolve was done.

    @return the string name. A value of \a nil indicates an error. 
*/
static VALUE 
lpsolve_get_col_name(VALUE self, VALUE column_num) 
{
  char *psz_col_name;
  INIT_LP;

  if (TYPE(column_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 1, is not an integer.\n", 
	   __FUNCTION__);
    return Qnil;
  }

  psz_col_name = get_col_name(lp, FIX2INT(column_num));
  return psz_col_name ? rb_str_new2(psz_col_name) : Qnil;
}

/** get_col_num(). - Not in API  (yet)

    get_col_num() returns the column number for the specified column
    name.  

    @return the column number. A value of \a nil indicates an error. 
*/
static VALUE 
lpsolve_get_col_num(VALUE self, VALUE column_name) 
{
  INIT_LP;
  
  if (TYPE(column_name) != T_STRING) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 1, is not a string.\n", 
	   __FUNCTION__);
    return Qnil;
  } else {
    int retval = get_col_num(lp, RSTRING_PTR(column_name));
    return (-1 == retval) ? Qnil : INT2FIX(retval);
  }
}

/** A wrapper for get_column().

  Get all column elements from the matrix for a given column number.

  @param self self

  @param column_num the column number that you want. The value must be
  between 1 and the number of columns.

  @return an array of the column values. The size of the array is 
  the number of rows + 1. \a Nil is returned if there was an error.
*/
static VALUE
lpsolve_get_column(VALUE self, VALUE column_num) 
{
  unsigned int i_column = FIX2INT(column_num);
  unsigned int i_rows;
  REAL *p_column;
  INIT_LP;
  
  i_rows =  get_Nrows(lp); /* Yep, Nrows, not Ncolumns. */
  p_column = ALLOC_N(REAL, i_rows + 1);
  
  if (!get_column(lp, i_column, p_column)) {
    free(p_column);
    return Qnil;
  } else {
    VALUE ret_ary = rb_ary_new2(i_rows+1);
    unsigned int i;
    for (i=0; i<=i_rows; i++) {
      rb_ary_push(ret_ary, rb_float_new(p_column[i]));
    }
    free(p_column);
    return ret_ary;
  }
  
}

/** A wrapper for get_infinite().

    @param self self
    @return Returns the value of "infinite".
*/
static VALUE lpsolve_get_infinite(VALUE self);
LPSOLVE_0_IN_NUM_OUT(get_infinite);

/** A wrapper for get_lowbo().

   Returns the upper bound on the variable identified by column.
   Setting a bound on a variable is the way to go instead of adding an
   extra constraint (row) to the model. Setting a bound doesn't
   increase the model size that means that the model stays smaller and
   will be solved faster.  The default upper bound of a variable is
   infinity (well not quite. It is a very big number. The value of
   lpsolve_get_infinite().

   @param self self

   @param column_num the column number of the variable. It must be
   between 1 and the number of columns in the lp.

*/

static VALUE lpsolve_get_lowbo(VALUE self, VALUE column_num);
LPSOLVE_1_IN_NUM_OUT(get_lowbo, T_FIXNUM, "an integer", FIX2INT);

/** A wrapper for get_lp_name().

    @return the name of the lp. The doc says the default name is
    "Unnamed", but in fact we seem to return "".
*/
static VALUE
lpsolve_get_lp_name(VALUE self) 
{
  char *psz_lp_name;
  INIT_LP;
  psz_lp_name = get_lp_name(lp);
  return psz_lp_name ? rb_str_new2(psz_lp_name) : rb_str_new2("Unnamed");
}

/** 
    A wrapper for get_Ncolumns().

    Return the number of columns (variables) in the lp object. Note
    that the number of columns can change when a presolve is done or
    when negative variables are split in a positive and a negative
    part.  Therefore it is advisable to use this function to determine
    how many columns there are in the lp instead of relying one's own
    count.

    @param self self
    @return the number of columns (variables) in the lp.
*/
static VALUE lpsolve_get_Ncolumns(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_Ncolumns)

/** A wrapper for get_nonzeros().

    Return the number of non-zero elements in matrix of the the lp object.

    @param self self
    @return the number of non-zero elements in the matrix of the lp.
*/
static VALUE lpsolve_get_nonzeros(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_nonzeros)

/** 
    A wrapper for get_Norig_columns().

    Return the number of original columns (variables) in the lp
    object. Note that the number of row does not change, but the
    number of original columns does not.

    @param self self
    @return the number of columns (variables) in the lp.
*/
static VALUE lpsolve_get_Norig_columns(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_Norig_columns)

/** A wrapper for get_Norig_rows().

    Return the number of original rows (constraints) in the lp
    object. Note that the number of row does not change, but the
    number of original rows does not.

    @param self self
    @return the number of original rows (constraints) in the lp.
*/
static VALUE lpsolve_get_Norig_rows(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_Norig_rows)

/** A wrapper for get_Nrows().

    Return the number of rows (variables) in the lp object. Note that
    the number of columns can change when a presolve is done or when
    negative variables are split in a positive and a negative part.
    Therefore it is advisable to use this function to determine how
    many columns there are in the lp instead of relying one's own
    count.

    @param self self
    @return the number of rows (constraints) in the lp.
*/
static VALUE lpsolve_get_Nrows(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_Nrows)

/** A wrapper for get_mat().

  Get a single element from the matrix.

  @param self self
  @param row_num row number
  @param col_num column number

  @return the value of the element on row \a row, column \a column. If no
  value was set for this element, the function returns 0.  Note that
  row entry mode must be off, else this function also fails. @see
  lpsolve_set_add_rowmode().

*/
static VALUE
lpsolve_get_mat(VALUE self, VALUE row_num, VALUE col_num) 
{
  INIT_LP;
  if (TYPE(row_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: row number, parameter 1, is not a number.\n", __FUNCTION__);
    return Qnil;
  }
  if (TYPE(col_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 2, is not a number.\n", __FUNCTION__);
    return Qnil;
  }
  return rb_float_new(get_mat(lp, FIX2INT(row_num), FIX2INT(col_num)));
}

/** A wrapper for get_mip_gap().

   Sets the MIP gap that specifies a tolerance for the branch and bound
   algorithm. This tolerance is the difference between the best-found
   solution yet and the current solution. If the difference is smaller
   than this tolerance then the solution (and all the sub-solutions) is
   rejected. This can result in faster solving times, but results in a
   solution which is not the perfect solution. So be careful with this
   tolerance.  The default mip_gap value is 1e-9


   @param self self
   @param abs_rel: set to true if the gap is absolute, or false for relative.
   @param val: gap value. The default mip_gap is 1e-9.
   @return \a gap value or nil if there was an error.
*/
static VALUE
lpsolve_get_mip_gap(VALUE self, VALUE abs_rel) 
{
  INIT_LP;
  if (abs_rel != Qtrue && abs_rel != Qfalse && abs_rel != Qnil) {
    report(lp, IMPORTANT,
	   "%s: Parameter is not a boolean or nil.\n",
	   __FUNCTION__);
    return Qnil;
  } else {
    return rb_float_new(get_mip_gap(lp, Qtrue == abs_rel));
  }
}

/** A wrapper for get_objective().

    returns the value of the objective of the last solve().
    This value is only valid after a successful lpsolve_solve() or 
    lag_solve.

    In Ruby, you can also use accessor function objective.

    @param self self
    @return the objective value. It is a real value.
*/
static VALUE lpsolve_get_objective(VALUE self);
LPSOLVE_0_IN_NUM_OUT(get_objective);

/** A wrapper for get_origcol_name().

    Returns the name of the specified column. 

    The difference between lpsolve_get_col_name() and
    lpsolve_get_origcol_name() is only visible when a presolve
    (lpsolve_set_presolve()) was done. Presolve can result in deletion
    of columns in the model. In lpsolve_get_col_name(), column
    specifies the column number after presolve was done.

    @param self self
    @param column_num column number

    @return nil indicates an error. 
*/
static VALUE
lpsolve_get_origcol_name(VALUE self, VALUE column_num) 
{
  lprec *lp;
  char *psz_col_name;
  Data_Get_Struct(self, lprec, lp);
  if (TYPE(column_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 1, is not an integer.\n", 
	   __FUNCTION__);
    return Qnil;
  }
  psz_col_name = get_origcol_name(lp, FIX2INT(column_num));
  return psz_col_name ? rb_str_new2(psz_col_name) : Qnil;
}

/** A wrapper for get_origrow_name()

    @param self self
    @param row_num row number

    get_row_name and get_origrow_name return the name of the specified
    row. A return value of NULL indicates an error. The difference
    between lpsolve_get_row_name() and lpsolve_get_origrow_name() is
    only visible when a presolve (lpsovle_set_presolve()) was
    done. Presolve can result in deletion of rows in the model. In
    get_row_name, row specifies the row number after presolve was
    done. In lpsolve_get_origrow_name(), row specifies the row number
    before presolve was done, ie the original row number. If presolve
    is not active then both functions are equal.

    @return a Ruby string row name.
*/
static VALUE
lpsolve_get_origrow_name(VALUE self, VALUE row_num) 
{
  char *psz_col_name;
  INIT_LP;
  if (TYPE(row_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: row number, parameter 1, is not a number.\n", __FUNCTION__);
    return Qnil;
  }
  psz_col_name = get_origrow_name(lp, FIX2INT(row_num));
  return psz_col_name ? rb_str_new2(psz_col_name) : Qnil;
}

/** 
    A wrapper for get_presolve().

    @param self self
    @return the number of columns (variables) in the lp.
*/
static VALUE lpsolve_get_presolve(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_presolve);

/** 
    A wrapper for get_presolveloops().

    Returns the number of times presolve is may be iterated. After a
    presolve is performed, another presolve may result in elimination
    of more rows and columns. This number specifies the maximum
    number of times this process may be repeated. 

    By default presolve repetition is performed until no simplification
    is done.

    @param self self
    @return maximum number of presolve loops. A value of -1 means we 
    loop for as many times as there is improvement.
*/
static VALUE lpsolve_get_presolveloops(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_presolveloops);

/** A wrapper for get_row().

  Get all row elements from the matrix for a given row number.

  @param self self

  @param row_num the row number that you want. The value must be
  between 1 and the number of rows.

  @return an array of the row values. The size of the array is 
  the number of columns + 1. Nil is returned if there was an error.
*/
static VALUE
lpsolve_get_row(VALUE self, VALUE row_num) 
{
  unsigned int i_row = FIX2INT(row_num);
  unsigned int i_columns;
  REAL *p_row;

  INIT_LP;

  if (TYPE(row_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: row number, parameter 1, is not a number.\n", __FUNCTION__);
    return Qnil;
  }
  i_columns =  get_Ncolumns(lp); /* Yep, Ncolumns, not Nrows. */
  p_row = ALLOC_N(REAL, i_columns + 1);
  
  if (!get_row(lp, i_row, p_row)) {
    free(p_row);
    return Qnil;
  } else {
    VALUE ret_ary = rb_ary_new2(i_columns+1);
    unsigned int i;
    for (i=0; i<=i_columns; i++) {
      rb_ary_push(ret_ary, rb_float_new(p_row[i]));
    }
    free(p_row);
    return ret_ary;
  }
  
}

/** A wrapper for get_row_name()

    @param self self
    @param row_num row number

    @return the name of the specified row. A return value of NULL
    indicates an error. The difference between lpsolve_get_row_name()
    and lpsolve_get_origrow_name() is only visible when a presolve
    (set_presolve) was done. Presolve can result in deletion of rows
    in the model. In lpsolve_get_row_name(), row specifies the row
    number after presolve was done. In lpsolve_aget_origrow_name(),
    row specifies the row number before presolve was done, ie the
    original row number. If presolve is not active then both functions
    are equal.  Returns a Ruby string row name.
*/
static VALUE
lpsolve_get_row_name(VALUE self, VALUE row_num) 
{
  char *psz_col_name;
  INIT_LP;
  if (TYPE(row_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: row number, parameter 1, is not a number.\n", __FUNCTION__);
    return Qnil;
  }
  psz_col_name = get_row_name(lp, FIX2INT(row_num));
  return psz_col_name ? rb_str_new2(psz_col_name) : Qnil;
}

/** A wrapper for get_scaling().

    @param self self
    @return A Ruby integer indicating scaling algorithm is used. 

    In Ruby, you can also use accessor function scaling.
*/
static VALUE lpsolve_get_scaling(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_scaling)

/** 
    A wrapper for get_simplextype()

    get_simplextype returns the desired combination of primal and dual
    simplex algorithms.

    The default is SIMPLEX_DUAL_PRIMAL.

    In Ruby, you can also use accessor function simplextype.

    @param self self

    @return a Ruby integer simplex type which is one of the following
    values:
       \a LPSolve::SIMPLEX_PRIMAL_PRIMAL, 
       \a LPSolve::DUAL_PRIMAL, 
       \a LPSolve::PRIMAL_DUAL, 
       \a LPSolve::DUAL_DUAL

*/
static VALUE lpsolve_get_simplextype(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_simplextype)

/** 
    A wrapper for get_solutioncount().

    Returns the number of equal solutions. This is only valid if there
    are integer, semi-continious or SOS variables in the model so that
    the branch-and-bound algoritm is used. This count gives the number
    of solutions with the same optimal objective value. If there is
    only one optimal solution, this value is 1.

    @param self self

    @return the number of equal solutions to to
    lpsolve_get_solutionlimit.
    
*/
static VALUE lpsolve_get_solutioncount(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_solutioncount)

/** 
    A wrapper for get_solutionlimit().

    returns the number of solutions that must be returned. 

    @param self self

    @return the number of solutions that must be returned.

    This is only valid if there are integer, semi-continious or SOS
    variables in the model so that the branch-and-bound algoritm is
    used. If there are more solutions with the same objective value,
    then this number specifies which solution must be returned. This
    can be used to retrieve all possible solutions. Start with 1 till
    get_solutioncount
    
*/
static VALUE lpsolve_get_solutionlimit(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_solutionlimit)

/** 
    return the status of the last solve.
    @param self self
    @return A return status number of the last solve.
*/
static VALUE lpsolve_get_status(VALUE self) 
{
  return rb_ivar_get(self, rb_intern("@status"));
}


/** 
    A wrapper for get_statustext(statuscode).
    However statuscode need not be supplied. If it isn't we use 
    the status code from the last solve.

    @param self self
    @param statuscode can be omitted or nil or an integer
    @return string description of statuscode

    In Ruby, you can also use accessor function statustext.
*/
static VALUE lpsolve_get_statustext(int argc, VALUE *argv, VALUE self)
{
  VALUE statuscode;
  unsigned int i_scanned;
  int i_statuscode;
  INIT_LP;
  if ((argc > 1) || (argc < 0))
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
  i_scanned = rb_scan_args(argc, argv, "01", &statuscode);
  switch (i_scanned) {
  case 0: 
    statuscode = rb_ivar_get(self, rb_intern("@status"));
    break;
  case 1: 
    if (statuscode != Qnil)
      statuscode = rb_ivar_get(self, rb_intern("@status"));
    else if (TYPE(statuscode) != T_FIXNUM) {
      report(lp, IMPORTANT, "%s: Parameter is not nil or an integer.\n",
	     __FUNCTION__);
      return Qnil;
    }
    break;
  default:
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", 
	     i_scanned);
  }

  i_statuscode = FIX2INT(statuscode);
  if (i_statuscode == SOLVE_NOT_CALLED)
    return rb_str_new2("LPSolve method solve() not performed yet.");
  else
    return rb_str_new2(get_statustext(lp, FIX2INT(statuscode)));
}

/** A wrapper for get_timeout().

    @param self self
    @return the number of seconds after which a timeout occurs.
*/
static VALUE lpsolve_get_timeout(VALUE self);
LPSOLVE_0_IN_NUM_OUT(get_timeout);

/** A wrapper for get_timeout().

    Returns the total number of iterations with Branch-and-bound of
    the last solution.  

    @param self self 

    @return the total number of iterations with Branch-and-bound of
    the last solution.  
*/
static VALUE lpsolve_get_total_iter(VALUE self);
LPSOLVE_0_IN_NUM_OUT(get_total_iter);

/** A wrapper for get_upbo().

   Returns the upper bound on the variable identified by column.
   Setting a bound on a variable is the way to go instead of adding an
   extra constraint (row) to the model. Setting a bound doesn't
   increase the model size that means that the model stays smaller and
   will be solved faster.  The default upper bound of a variable is
   infinity (well not quite. It is a very big number. The value of
   lpsolve_get_infinite().

   @param self self

   @param column_num the column number of the variable. It must be
   between 1 and the number of columns in the lp.

*/

static VALUE lpsolve_get_upbo(VALUE self, VALUE column_num);
LPSOLVE_1_IN_NUM_OUT(get_upbo, T_FIXNUM, "an integer", FIX2INT);

/** A wrapper for get_var_dualresult()

    

    @param self self
    @param index index 

    @return returns the reduced cost.

    In contrast to get_dual_solution, the original index number is
    preserved.
*/
static VALUE lpsolve_get_var_dualresult(VALUE self, VALUE index);
LPSOLVE_1_IN_NUM_OUT(get_var_dualresult, T_FIXNUM, "an integer", FIX2INT);

/** A wrapper for get_var_primalresult()

    @param self self
    @param index index 

    @return the name of the specified row. A return value of NULL
    indicates an error. The difference between lpsolve_get_row_name()
    and lpsovle_get_origrow_name() is only visible when a presolve
    (set_presolve) was done. Presolve can result in deletion of rows
    in the model. In lpsolve_get_row_name(), row specifies the row
    number after presolve was done. In lpsovle_aget_origrow_name(),
    row specifies the row number before presolve was done, ie the
    original row number. If presolve is not active then both functions
    are equal.  Returns a Ruby string row name.

    Retrieve the a value of the objective function, constraints and variables.
    These values are only valid after a successful solve or lag_solve.

    In contrast to get_primal_solution, the original index number is
    preserved.
*/
static VALUE lpsolve_get_var_primalresult(VALUE self, VALUE index);
LPSOLVE_1_IN_NUM_OUT(get_var_primalresult, T_FIXNUM, "an integer", FIX2INT);

/** A wrapper for get_variables().

    Get the values of the variables.
    @param self self
    
    @return an array of the variable values. The size of the array is 
    the number of columns + 1. Nil is returned if there was an error.
*/
static VALUE
lpsolve_get_variables(VALUE self) 
{
  unsigned int i_columns;
  REAL *p_variables;

  INIT_LP;
  i_columns =  get_Ncolumns(lp);
  
  if (!get_ptr_variables(lp, &p_variables)) {
    return Qnil;
  } else {
    VALUE ret_ary = rb_ary_new2(i_columns);
    unsigned int i;
    for (i=0; i<i_columns; i++) {
      rb_ary_push(ret_ary, rb_float_new(p_variables[i]));
    }
    return ret_ary;
  }
  
}

/** A wrapper for get_verbose()

    get_verbose returns the current verbose level. Can be one of the
    following values:
       \a LPSolve::NEUTRAL,   \a LPSolve::CRITICAL, \a LPSolve::SEVERE,
       \a LPSolve::IMPORTANT, \a LPSolve::NORMAL,   \a LPSolve::DETAILED, or
       \a LPSolve::FULL

    How much information is reported depends on the verbose level. The
    default verbose level is \a LPSOLVE::NORMAL. lp_solve determines
    how verbose a given message is. For example specifying a wrong
    row/column index values is considered as a \a LPSOLVE::SEVERE
    error. All messages equal to and below the set level are reported.
    The default reporting device is the console screen. It is possible
    to set a used defined reporting routine via lpsolve_put_logfunc().

    In Ruby, you can also use accessor function verbose.

    @param self self
    @return a Ruby integer verbosity level.

*/
static VALUE lpsolve_get_verbose(VALUE self);
LPSOLVE_0_IN_INT_OUT(get_verbose)

static VALUE
lpsolve_initialize(VALUE self, VALUE num_vars, VALUE num_constraints) 
{
  int i_vars = NUM2INT(num_vars);
  int i_constraints = NUM2INT(num_constraints);
  lprec *lp = make_lp(i_vars, i_constraints);
  DATA_PTR(self) = lp;
  rb_ivar_set(self, rb_intern("@status"), INT2FIX(SOLVE_NOT_CALLED));
  return self;
}

/** A wrapper for is_debug().

    Returns a flag if all intermediate results and the
    branch-and-bound decisions must be printed while solving. This
    function is meant for debugging purposes. The default is not to
    debug.

    Returns a flag if 
    @param self self

    @return true, false, or nil on error. If true is returned, all
    intermediate results and the branch-and-bound decisions will be
    printed while solving.
*/
static VALUE lpsolve_is_debug(VALUE self);
LPSOLVE_0_IN_BOOL_OUT(is_debug);

/** A wrapper for is_maxim().

    @param self self
    @return boolean or nil on error.
*/
static VALUE lpsolve_is_maxim(VALUE self);
LPSOLVE_0_IN_BOOL_OUT(is_maxim);

/** A wrapper for is_SOS_var().

    Returns if a variable is a SOS variable or not. By default a variable
    is not SOS. A variable becomes a SOS variable via lpsolve_add_SOS().

    @param column the column number of the variable to be checked. It
    should be between 1 and the number of columns in the lp.
    @param self self
    @return boolean or nil on error.
*/
static VALUE lpsolve_is_SOS_var(VALUE self, VALUE column);
LPSOLVE_1_IN_BOOL_OUT(is_SOS_var, T_FIXNUM, "an integer", FIX2INT)

static void __WINAPI
lpsolve_logfunction(lprec *lp, void *userhandle, char *buf)
{
  printf("***%s\n", buf);
}

/**
   Constructs a new LP. Sets all variables to initial values.  The LP
   has rows rows and columns columns. The matrix contains no values,
   but space for one value. All arrays that depend on rows and columns
   are allocated.

   @param class_or_model_name either the class or module name which this
   is to belong to (it will be equal to LPSolve or LPsolve). If you are 
   calling from Ruby, this parameter will be taken care of automatically.
   @param num_vars number of variables or columns
   @param num_constraints number of constraints or rows

   @return nil is returned if there was an error.
*/

static VALUE
lpsolve_make_lp(VALUE class_or_model_name, VALUE num_constraints, 
		VALUE num_vars) 
{
  int i_constraints = NUM2INT(num_constraints);
  int i_vars = NUM2INT(num_vars);
  lprec *lp = make_lp(i_constraints, i_vars);
  if (NULL == lp) {
    return Qnil;
  } else {
    VALUE obj = lpsolve_alloc(rb_cLPSolve);
    DATA_PTR(obj) = lp;
    return obj;
  }
}

/** A wrapper for print_duals(). 

    @param self self
    @return \a true unless we have an error.
*/
LPSOLVE_0_IN_STATUS_OUT(print_duals)

/** A wrapper for print_debugdump(). 

    The print_debugdump function creates a generic readable data dump
    of key lp_solve model variables; principally for run difference and
    debugging purposes This function is meant for debugging purposes.

    @return \a true if we could write the output file.
*/
static VALUE lpsolve_print_debugdump(VALUE self, VALUE filename);
LPSOLVE_1_STR_IN_BOOL_OUT(print_debugdump)

/** A wrapper for print_lp. 

    @param self self
    @return \a true unless we have an error.
*/
static VALUE lpsolve_print_lp(VALUE self);
LPSOLVE_0_IN_STATUS_OUT(print_lp)

static VALUE lpsolve_print(VALUE self);
void print(lprec *lp);
LPSOLVE_0_IN_STATUS_OUT(print)

/** A wrapper for print_constraints. 
    @param self self
    @param num constraint number.
    @return \a true unless we have an error.
*/
static VALUE lpsolve_print_constraints(VALUE self, VALUE num);
LPSOLVE_1_IN_STATUS_OUT(print_constraints, FIX2INT(param1));

/** A wrapper for print_logfunc
    @return nil.
*/
static VALUE
lpsolve_put_logfunc(VALUE self, VALUE logfunc_name) 
{
  lprec *lp;
  rb_define_readonly_variable("@logfunc_name", &logfunc_name);
  Data_Get_Struct(self, lprec, lp);
  put_logfunc(lp, lpsolve_logfunction, NULL);
  return Qnil;
}

/** A wrapper for print_objective. 
    @return nil.
*/
LPSOLVE_0_IN_STATUS_OUT(print_objective)

/** A wrapper for print_tableau(). 

    The print_tableau function prints the tableau. This function only
    works after a successful solve This function is meant for
    debugging purposes. By default, the output is stdout. However this
    can be changed via a call to set_outputstream, set_outputfile.

    @param self self
    @return \a true unless we have an error.
*/
LPSOLVE_0_IN_STATUS_OUT(print_tableau)

/** A wrapper for read_LP. 

  Create a LPSolve object and read an lp model from file.

  Returns anew LPSolve object. A Nil return value indicates an
  error. Specifically file could not be opened or file has wrong
  structure or not enough memory available to setup an lprec
  structure.

*/
static VALUE
lpsolve_read_LP(VALUE model, VALUE filename, VALUE verbosity, 
		VALUE model_name) 
{
  lprec *lp;

  if (TYPE(filename) != T_STRING) {
    return Qnil;
  }
  
  if (TYPE(verbosity) != T_FIXNUM) {
    return Qnil;
  }
  
  if (TYPE(model_name) != T_STRING) {
      return Qnil;
  }
  
  lp = read_LP(RSTRING_PTR(filename), verbosity, RSTRING_PTR(model_name));
  if (NULL == lp) {
    return Qnil;
  } else {
    VALUE obj = lpsolve_alloc(rb_cLPSolve);
    DATA_PTR(obj) = lp;
    return obj;
  }
}

/** A wrapper for read_MPS.

  Create a LPSolve object and read an MPS model from a file.

  Returns a new LPSolve Object. A Nil return value indicates an
  error. Specifically file could not be opened or file has wrong
  structure or not enough memory available to setup an lprec
  structure.

*/
static VALUE
lpsolve_read_MPS(VALUE module, VALUE filename, VALUE verbosity) 
{
  lprec *lp;

  if (TYPE(filename) != T_STRING) {
    return Qnil;
  }
  
  if (TYPE(verbosity) != T_FIXNUM) {
      return Qnil;
  }
  
  lp = read_MPS(RSTRING_PTR(filename), verbosity);
  if (NULL == lp) {
    return Qnil;
  } else {
    VALUE obj = lpsolve_alloc(rb_cLPSolve);
    DATA_PTR(obj) = lp;
    return obj;
  }
}

/** A wrapper for print_str

    Prints a string. By default, the output is stdout. However this
    can be changed via a call to lpsolve_set_outputfile().  Possibly
    useful for debugging/demo purposes.

    @param self self 
    @param str the string to print

    @return \a true if string was printed; false on error such as the
    str parameter is not a string type.
*/
static VALUE
lpsolve_print_str(VALUE self, VALUE str)
{
  lprec *lp;
  Data_Get_Struct(self, lprec, lp);

  if (TYPE(str) != T_STRING) {
    report(lp, IMPORTANT, 
	   "%s: parameter 2 is not a string.\n",
	   __FUNCTION__);
    return Qfalse;
  }
  
  print_str(lp, RSTRING_PTR(str));
  return Qtrue;
}

/** A wrapper for print_solution. 

    Prints the solution (variables) of the lp. This can only be done
    after a successful solve.  This function is meant for debugging
    purposes. By default, the output is stdout. However this can be
    changed via a call to lpsolve_set_outputfile().

    @param self self
    @param columns Number of columns to use in printing the solution.
    if columns is negative we will only print those variables which are
    nonzero.

    @return \a true unless we have an error.
*/
static VALUE lpsolve_print_solution(VALUE self, VALUE columns);
static VALUE
lpsolve_print_solution(VALUE self, VALUE columns) 
{
  lprec *lp;
  int i_columns = FIX2INT(columns);
  Data_Get_Struct(self, lprec, lp);
  if (NULL == lp) return Qfalse;
  if (i_columns < 0) {
    int print_sol_save = lp->print_sol;
    lp->print_sol = lp->print_sol | AUTOMATIC;
    print_solution(lp, -i_columns);
    lp->print_sol = print_sol_save;
  } else {
    print_solution(lp, i_columns);
  }
  return Qtrue;
}

/** wrapper for set_bb_depthlimit

Sets the maximum branch-and-bound depth.

Parameters

lp

Pointer to previously created lp model. See return value of make_lp, copy_lp, read_lp, read_LP, read_mps, read_freemps, read_MPS, read_freeMPS, read_XLI

bb_maxlevel

Specifies the maximum branch-and-bound depth. A positive value means
that the depth is absoluut. A negative value means a relative B&B
depth limit. The "order" of a MIP problem is defined to be 2x the
number of binary variables plus the number of SC and SOS variables. A
relative value of -x results in a maximum depth of x times the order
of the MIP problem.

Remarks

The set_bb_depthlimit function sets the maximum branch-and-bound
depth.  This is only useful if there are integer, semi-continious or
SOS variables in the model so that the branch-and-bound algorithm must
be used to solve them. The branch-and-bound algorithm will not go
deeper than this level. When 0 then there is no limit to the
depth. Limiting the depth will speed up solving time, but there is a
chance that the found solution is not the most optimal one. Be aware
of this. It can also result in not finding a solution at all.  The
default is -50.
*/
static VALUE lpsolve_set_bb_depthlimit(VALUE self, VALUE limit);
LPSOLVE_1_IN_STATUS_OUT(set_bb_depthlimit, FIX2INT(param1));

/** wrapper for set_bb_bb_rule

The set_bb_rule function specifies the branch-and-bound rule for
choosing which non-integer variable is to be selected. This rule can
influence solving times considerably. Depending on the model one rule
can be best and for another model another rule.  The default is
NODE_FIRSTSELECT (0).

 */
static VALUE lpsolve_set_bb_rule(VALUE self, VALUE bb_rule);
LPSOLVE_1_IN_STATUS_OUT(set_bb_rule, FIX2INT(param1));

/** A wrapper for set_add_rowmode().

    Normally a model is built either column by column or row by row.
    
    The default \a on_off setting is \a false, which assumes model
    building column by column.  So lpsolve_add_column(),
    lpsolve_add_columnex(), and lpsolve_str_add_column() perform best
    here.

    If the model is built row by row via lpsolve_add_constraint(),
    lpsolve_add_constraintex(), or lpsolve_str_add_constraint() calls,
    then these routines will be much faster if this routine is called
    with \a on_off set \a true. The speed improvement is spectacular,
    especially for bigger models, so it is advisable to call this
    routine to set the mode.

    There are several restrictions with this mode. Only use this
    function after lpsolve_make_lp() is called, not when the model is
    read from a file. Also, if this function is used, first add the
    objective function via set_obj_fn, set_obj_fnex, str_set_obj_fn
    and after that add the constraints via add_constraint,
    add_constraintex, str_add_constraint. Don't call other API
    functions while in row entry mode. No other data matrix access is
    allowed while in row entry mode. After adding the contraints, turn
    row entry mode back off. Once turned off, you cannot switch back to
    row entry mode. So in short: - turn row entry mode on - set the
    objective function - create the constraints - turn row entry mode
    off

    @param self self
    @param on_off \a true if are going to add by rows, \a false if not
    or addign by columns.

    @return \a true if changed from mode and \a false if this mode was
    already set; \a nil if there was an error
*/
static VALUE lpsolve_set_add_rowmode(VALUE self, VALUE on_off);
LPSOLVE_1_BOOL_IN_BOOL_OUT(set_add_rowmode);

/** A wrapper for set_binary().

    Set the type of the variable to be binary or floating point.
    The default type of a variable is floating point. 

    @param self self

    @param column_num column number.

    @param new_bool \a true if you want the column (variable) to take
    on a binary value, \a false or \a nil if you want the column to
    take on a floating-point value. If no value is given we assume \a
    true (set to binary).

    @return \a true if the operation was successful, \a false if there
    was an error.
*/
#if 0
LPSOLVE_SET_VARTYPE(set_binary)
#endif

/** A wrapper for set_bounds().

    The set_bounds function sets a lower and upper bound on the
    variable identified by column.  Setting a bound on a variable is
    the way to go instead of adding an extra constraint (row) to the
    model. Setting a bound doesn't increase the model size that means
    that the model stays smaller and will be solved faster.  Note that
    the default lower bound of each variable is 0. So variables will
    never take negative values if no negative lower bound is set. The
    default upper bound of a variable is infinity (well not quite. It
    is a very big number. The value of get_infinite).

   @return \a true if no errors.
*/
static VALUE
lpsolve_set_bounds(VALUE self, VALUE column_num, VALUE lower_bound, 
		   VALUE upper_bound)
{
  INIT_LP;
  if (TYPE(column_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 1, is not an integer.\n",
	   __FUNCTION__);
    return Qfalse;
  }

  RETURN_BOOL(set_bounds(lp, FIX2INT(column_num), 
			 NUM2DBL(lower_bound), NUM2DBL(upper_bound)));
}

/** A wrapper for set_col_name
    Returns a Ruby string column name.
*/
static VALUE
lpsolve_set_col_name(VALUE self, VALUE column_num, VALUE new_name) 
{
  INIT_LP;

  if (TYPE(column_num) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 1, is not an integer.\n",
	   __FUNCTION__);
    return Qnil;
  }

  if (TYPE(new_name) != T_STRING) {
    report(lp, IMPORTANT, 
	   "%s: new name, parameter 2, is not a string.\n",
	   __FUNCTION__);
    return Qnil;
  }

  return set_col_name(lp, FIX2INT(column_num), RSTRING_PTR(new_name)) ?
    Qtrue: Qfalse;
}

/** A wrapper for set_debug

    Sets a flag if all intermediate results and the branch-and-bound
    decisions must be printed while solving.

    @return \a true if no errors, false if an error (e.g. you didn't
    pass true or false).
*/
static VALUE
lpsolve_set_debug(VALUE self, VALUE new_bool) 
{
  if (new_bool != Qtrue && new_bool != Qfalse) {
    return Qfalse;
  } else {
    INIT_LP;
    set_debug(lp, Qtrue == new_bool);
    return Qtrue;
  }
}

/** A wrapper for set_int

    Set the type of the variable to be integer or floating point.
    The default type of a variable is floating point. 

    The argument \a new_bool specifies what the status of the variable
    becomes. From the moment there is at least one integer variable in
    the model, the Branch and Bound algorithm is used to make these
    variables integer. Note that solving times can be considerably
    larger when there are integer variables. 

    @param self self

    @param column_num column number of variable

    @param new_bool \a true if you want the column (variable) to be an
    integer, \a false or \a nil if you want the column not to take on
    a floating-point value.  If no parameter is given, we assumed it
    to be true (set to integer).

    @param new_bool

    @return \a true if the operation was successful, \a false if there
    was an error.
*/
LPSOLVE_SET_VARTYPE(set_int)

/** A wrapper for set_lp_name

    Set the name of the lp. Returns true if the operation was
    successful. A return value of false indicates an error.
*/
static VALUE lpsolve_set_lp_name(VALUE self, VALUE model_name);
LPSOLVE_1_STR_IN_BOOL_OUT(set_lp_name)

/** A wrapper for set_lowbo().
    @return \a true if no errors.
*/
static VALUE
lpsolve_set_lowbo(VALUE self, VALUE column, VALUE val) 
{
  INIT_LP;
  if (TYPE(column) != T_FIXNUM) {
    report(lp, IMPORTANT, 
	   "%s: column number, parameter 1, is not an integer.\n",
	   __FUNCTION__);
    return Qnil;
  }
  RETURN_BOOL(set_lowbo(lp, FIX2INT(column), NUM2DBL(val)));
}

/** A wrapper for set_mat().
    @return \a true if no errors.
*/
static VALUE
lpsolve_set_mat(VALUE self, VALUE row, VALUE column, VALUE val) 
{
  INIT_LP;
  RETURN_BOOL(set_mat(lp, FIX2INT(row), FIX2INT(column), NUM2DBL(val)));
}

/** A wrapper for set_maxim().

    Sets the objective direction to maximize. The default is to
    minimize, except when reading a model in via lpsolve_read_LP. 

    @return \a true unless we have an error.
*/
static VALUE lpsolve_set_maxim(VALUE self);
LPSOLVE_0_IN_STATUS_OUT(set_maxim)

/** A wrapper for set_minim().

    Sets the objective direction to minimize. The default is to
    minimize, except when reading a model in via lpsolve_read_LP. 

    @return \a true unless we have an error.
*/
static VALUE lpsolve_set_minim(VALUE self);
LPSOLVE_0_IN_STATUS_OUT(set_minim)

/** A wrapper for set_mip_gap().

   Sets the MIP gap that specifies a tolerance for the branch and bound
   algorithm. This tolerance is the difference between the best-found
   solution yet and the current solution. If the difference is smaller
   than this tolerance then the solution (and all the sub-solutions) is
   rejected. This can result in faster solving times, but results in a
   solution which is not the perfect solution. So be careful with this
   tolerance.  The default mip_gap value is 1e-9


   @param self self
   @param abs_rel: set to true if the gap is absolute, or false for relative.
   @param val: gap value. The default mip_gap is 1e-9.
   @return \a true if no errors.
*/
static VALUE
lpsolve_set_mip_gap(VALUE self, VALUE abs_rel, VALUE val) 
{
  INIT_LP;
  if (abs_rel != Qtrue && abs_rel != Qfalse && abs_rel != Qnil) {
    report(lp, IMPORTANT,
	   "%s: Parameter is not a boolean or nil.\n",
	   __FUNCTION__);
    return Qfalse;
  } else {
    set_mip_gap(lp, Qtrue == abs_rel, NUM2DBL(val));
    return Qtrue;
  }
}

/** A wrapper for str_set_obj_fn().

    Set the objective function (row 0) of the matrix.

    @return \a true unless we have an error, then \a nil or \a false.
*/
static VALUE
lpsolve_set_obj_fnex(VALUE self, VALUE row_coeffs) 
{
  REAL *row = NULL;
  int *colno = NULL;
  long int i, count;
  VALUE ret = Qnil;
  VALUE *p_row_coeff = NULL;

  INIT_LP;

  /***FIXME: combine common parts of this with add_constraintex ****/
  if (TYPE(row_coeffs) != T_ARRAY) {
    report(lp, IMPORTANT, 
	   "%s: row coefficients parameter is not an array.\n",
	   __FUNCTION__);
    return Qnil;
  }

  count   = RARRAY_LEN(row_coeffs);
  colno   = ALLOC_N(int, count);
  row     = ALLOC_N(REAL, count);

  p_row_coeff = RARRAY_PTR(row_coeffs);
  for (i = 0; i < count; i++) {
    int i_col;
    if (TYPE(*p_row_coeff) != T_ARRAY) {
      report(lp, IMPORTANT, 
	     "%s: row coeffient element %d is not an array.\n", 
	     __FUNCTION__, i);
      goto done;
    }
    if (RARRAY_LEN(*p_row_coeff) != 2) {
      report(lp, IMPORTANT, 
	     "%s: row coeffient element %d is not an array tuple.\n", 
	     __FUNCTION__, i);
      goto done;
    } else {
      VALUE *tuple = RARRAY_PTR(*p_row_coeff);
      if (TYPE(tuple[0]) != T_FIXNUM) {
	report(lp, IMPORTANT, 
	       "%s: Column number, first element, of row coefficients at " \
	       "tuple %d is not a integer.\n", __FUNCTION__, i);
	goto done;
      }
      switch (TYPE(tuple[1])) {
      case T_FIXNUM:
      case T_FLOAT: break;
      default:
	report(lp, IMPORTANT, 
	       "%s: Coefficient value, second element, of row coeffients at "
	       "tuple %d is not an integer.\n", __FUNCTION__, i);
	goto done;
      }

      i_col = FIX2INT(tuple[0]);
      if (i_col <= 0 || i_col > lp->columns) {
	report(lp, IMPORTANT, 
	       "%s: Column number, first element, of row coeffients at " \
	       "tuple %d is in the range 1..%d\n", 
	       __FUNCTION__, i, lp->columns);
	goto done;
      }
      colno[i] = i_col;
      row[i]   = NUM2DBL(tuple[1]);
    }
    p_row_coeff++;
  }

  ret = set_obj_fnex(lp, count, row, colno) ? Qtrue : Qfalse ;

 done:
  free(row);
  free(colno);
  return ret;

}

/** A wrapper for set_outputfile(). 

    @return \a true if we could set the output file.
*/
static VALUE lpsolve_set_outputfile(VALUE self, VALUE filename);
LPSOLVE_1_STR_IN_BOOL_OUT(set_outputfile)

/** A wrapper for set_presolve(). 

    @return \a true if no errors.
*/
static VALUE lpsolve_set_presolve(VALUE self, VALUE do_presolve, 
				  VALUE maxloops) 
{
  INIT_LP;
  if (TYPE(do_presolve) != T_FIXNUM) {
    report(lp, IMPORTANT, "%s: Presolve parameter is not a number (bitmask).\n",
	   __FUNCTION__);
    return Qnil;
  }
  if (TYPE(maxloops) != T_FIXNUM ) {
    report(lp, IMPORTANT, 
	   "%s: maxloops parameter should be a number (bitmask).\n",
	   __FUNCTION__);
    return Qnil;
  }
  set_presolve(lp, FIX2INT(do_presolve), FIX2INT(maxloops));
  return Qtrue;
}


/** A wrapper for set_presolve(). 

    @return \a true if no errors.
*/
static VALUE lpsolve_set_presolve1(VALUE self, VALUE do_presolve)
{
  int i_maxloops;
  INIT_LP;
  if (TYPE(do_presolve) != T_FIXNUM) {
    report(lp, IMPORTANT, "%s: Presolve parameter is not a number (bitmask).\n",
	   __FUNCTION__);
    return Qnil;
  }
  i_maxloops = get_presolveloops(lp);
  set_presolve(lp, FIX2INT(do_presolve), i_maxloops);
  return Qtrue;
}


/** A wrapper for set_rh().

    (Re)Set the value of the right hand side (RHS) vector (column 0) for 
    the specified row.

    @param self self
    @param row_num the row number for which the RHS is to be must be set. 
    Must be between 0 and number of rows in the lp.

    @param value The value to set the RHS to.

    @return \a true of no error, or \a nil if there was an error.
*/
static VALUE
lpsolve_set_rh(VALUE self, VALUE row_num, VALUE value) 
{
  INIT_LP;
  set_rh(lp, FIX2INT(row_num), NUM2DBL(value));
  return Qnil;
}

/** A wrapper for set_rh_range().

   Set a range on the constraint (row) identified by row.  Setting a
   range on a row is the way to go instead of another constraint (row) to
   the model. (Note that in either case, there are two API calls 
   to set the lower and upper bound. 

   Setting a range doesn't increase the model size, and that means
   that the model stays smaller and will be solved faster.  If the row
   previously had a less-than constraint, then the range means setting
   a minimum on the constraint that is equal to the RHS value minus
   the range. If the row has a greater-than constraint then the range
   means setting a maximum on the constraint that is equal to the RHS
   value plus the range.  Note that the range value is the difference
   value and not the absolute value.  Set the value of the right hand
   side (RHS) vector (column 0) for the specified row.

    @param self self
    @param row_num the row number for which the RHS is to be must be set. 
    Must be between 0 and number of rows in the lp.

    @param value The value to set the RHS to.

    @return \a true of no error, or \a nil if there was an error.
*/
static VALUE
lpsolve_set_rh_range(VALUE self, VALUE row_num, VALUE deltavalue) 
{
  INIT_LP;
  RETURN_BOOL(set_rh_range(lp, FIX2INT(row_num), NUM2DBL(deltavalue)));
}

/** A wrapper for set_row_name.

    Set the name of a constraint (row) in the lp.

    @return \a true if the operation was successful, false if not.
*/
static VALUE 
lpsolve_set_row_name(VALUE self, VALUE row_num, VALUE new_name) 
{
  INIT_LP;

  if (TYPE(new_name) != T_STRING) {
    return Qfalse;
  }

  RETURN_BOOL(set_row_name(lp, FIX2INT(row_num), RSTRING_PTR(new_name)));
}

/** A wrapper for set_semicont().

Set the type of the variable (column) to be semi-continuous or not. By
default, a variables are not semi-continuous. Note that a
semi-continuous variable should have a nonzero lower bound to for this
to have different effect. Recall that the default lower bound on
variables is zero. The lower bound may be set before or after setting
the semi-continuous status. 

    @param self self

    @param column_num column number of the variable

    @param new_bool true if you want the column (variable) to be a
    semi-continuous value, \a false or \a nil if you want the column
    not to take on a floating-point value. If no parameter specified,
    \a true is assumed

    @return \a true if the operation was successful, \a false if there
    was an error.
*/
LPSOLVE_SET_VARTYPE(set_semicont)

/** 
    return the status status code of the last solve.
    @param self self
    @return A return status number of the last solve.
*/
static VALUE lpsolve_set_status(VALUE self, VALUE newstatus) 
{
  INIT_LP;
  if (FIXNUM_P(newstatus))
    return rb_ivar_set(self, rb_intern("@status"), newstatus);
  else
    report(lp, IMPORTANT, 
	   "%s: status should be a Fixnum.\n", __FUNCTION__);
  return Qnil;
}


/** A wrapper for set_timeout().

    In Ruby, you can also use accessor function timeout=.

    The solve() and lag_solve() methods may not last longer than this
    time or the routines return with a timeout. The default timeout is
    0, resulting in no timeout.  If a timout occurs, but there was
    already an integer solution found (that is possibly not the best),
    then solve will return LPSolve::SUBOPTIMAL. If there was no integer
    solution found yet or there are no integers or the solvers is
    still in the first phase where a REAL optimal solution is searched
    for, then solve will return TIMEOUT.

    Set a timeout for solving .
    @param self self
    @param sec_timeout floating-point timeout value in seconds.
    @return nil.
*/
static VALUE
lpsolve_set_timeout(VALUE self, VALUE sec_timeout) 
{
  INIT_LP;
  set_timeout(lp, NUM2DBL(sec_timeout));
  return Qnil;
}

/** A wrapper for set_scaling().

    Specifies which scaling algorithm must be used. 

    @return nil.
*/
static VALUE
lpsolve_set_scaling(VALUE self, VALUE new_scalemode) 
{
  INIT_LP;
  set_scaling(lp, FIX2INT(new_scalemode));
  return Qnil;
}

/** A wrapper for set_simplextype().

    In Ruby, you can also use accessor function simplextype=.

    @param self
    @param new_simplextype desired level of verbosity, an integer. @see 
    lpsolve_get_simplextype() for simplex types.
    @return nil on error. 
*/
static VALUE lpsolve_set_simplextype(VALUE self, VALUE new_simplextype);
LPSOLVE_1_IN_STATUS_OUT(set_simplextype, FIX2INT(param1))

/** A wrapper for set_solutionlimit().

    Sets the solution number that must be returned.

    This function is only valid if there are integer, semi-continious
    or SOS variables in the model so that the branch-and-bound
    algoritm is used. If there are more solutions with the same
    objective value, then this number specifies which solution must be
    returned. This can be used to retrieve all possible
    solutions. Start with 1 till get_solutioncount 

    @param self
    @param limit number of solutions needed
    @return nil on error. 

@return nil.
*/
static VALUE lpsolve_set_solutionlimit(VALUE self, VALUE limit);
LPSOLVE_1_IN_STATUS_OUT(set_solutionlimit, FIX2INT(param1))

/** A wrapper for set_trace

    Sets a flag if pivot selection must be printed while solving.

    @param self self
    @param print_bool print if true; don't print if false.
    @return \a true if the operation was successful, FALSE if there was an error.
*/
static VALUE
lpsolve_set_trace(VALUE self, VALUE print_bool) 
{
  if (print_bool != Qtrue && print_bool != Qfalse) {
    return Qfalse;
  } else {
    INIT_LP;
    set_trace(lp, Qtrue == print_bool);
    return Qtrue;
  }
}

/** A wrapper for set_upbo().

    @return \a true if no errors.
*/
static VALUE
lpsolve_set_upbo(VALUE self, VALUE column, VALUE val) 
{
  INIT_LP;
  RETURN_BOOL(set_upbo(lp, FIX2INT(column), NUM2DBL(val)));
}

/** A wrapper for set_verbose().

    In Ruby, you can also use accessor function verbose=.

    @param self
    @param new_verbosity desired level of verbosity, an integer. @see 
    lpsolve_get_verbose() for verbosity levels.
    @return \a true unless we have an error.
*/
static VALUE lpsolve_set_verbose(VALUE self, VALUE new_verbosity);
LPSOLVE_1_IN_STATUS_OUT(set_verbose, FIX2INT(param1))

/** A wrapper for solve().
    @returns 0 if no error.
*/

static VALUE lpsolve_solve(VALUE self) 
{
  INIT_LP;
  if (NULL != lp) { 
    VALUE status = INT2FIX(solve(lp));
    rb_ivar_set(self, rb_intern("@status"), status);
    return status;
  } else {
    return Qnil;
  }
}

/** A wrapper for str_add_column().

    @return \a true if the operation was successful. A false value
    indicates an error.
*/
static VALUE lpsolve_str_add_column(VALUE self, VALUE col_str);
LPSOLVE_1_STR_IN_BOOL_OUT(str_add_column)

/** A wrapper for str_add_constraint().
    @return boolean
*/
static VALUE
lpsolve_str_add_constraint(VALUE self, VALUE constraint, 
			  VALUE compare, VALUE num_constraints) 
{
  char *psz_constraint = RSTRING_PTR(constraint);
  int  i_compare = NUM2INT(compare);
  int  i_constraints = NUM2INT(num_constraints);
  lprec *lp;
  Data_Get_Struct(self, lprec, lp);
  RETURN_BOOL(str_add_constraint(lp, psz_constraint, i_compare, i_constraints));
}

/** A wrapper for str_set_obj_fn().

    Set the objective function (row 0) of the matrix via a string.

    @return \a true unless we have an error.
*/
static VALUE
lpsolve_str_set_obj_fn(VALUE self, VALUE obj_fn) 
{
  char *psz_obj_fn = RSTRING_PTR(obj_fn);
  lprec *lp;
  Data_Get_Struct(self, lprec, lp);
  RETURN_BOOL(str_set_obj_fn(lp, psz_obj_fn));
}

/** A wrapper for time_elapsed(). 

    The time_elapsed function returns the time in seconds since solve
    and lag_solve has started. In contrast to totaltime, the time does
    not include the load time. If solving has not completed he value
    will be the number of seconds of until the time of the call
    rather than the time the solve completed.

    @param  self self.

    @return \a Floating-point number for time since solve and
    lag_solve has started
*/
static VALUE lpsolve_time_elapsed(VALUE self);
LPSOLVE_0_IN_NUM_OUT(time_elapsed);

/** \fn time_load Return the amount of time used to load in data

    @param self self

    @return A Ruby floating-point number indicating how long the
    presolver solver took.

*/
static VALUE lpsolve_time_load(VALUE self) ;
LPSOLVE_0_IN_TIME_OUT(time_load, timecreate, timestart);

/** Return the amount of time used in the simplex solver

    @param self self

    @return A Ruby floating-point number indicating how long the
    simplex solver took.

    This only makes sense if a problem was solved. See also
    the status instance variable.

    In Ruby, you can also use accessor function simplextime
*/
static VALUE lpsolve_time_simplex(VALUE self) ;
LPSOLVE_0_IN_TIME_OUT(time_simplex, timepresolved, timeend);

/** \fn time_presolve
    Return the amount of time used in the presolver

    @param self self

    @return A Ruby floating-point number indicating how long the
    presolver solver took.

    This only makes sense if a problem was solved. See also
    the status instance variable.

    In Ruby, you can also use accessor function simplextime
*/
static VALUE lpsolve_time_presolve(VALUE self) ;
LPSOLVE_0_IN_TIME_OUT(time_presolve, timestart, timepresolved);

/** Return the total amount elapsed time from start to finish.

    @param self self

    @return A Ruby floating-point number indicating how long the
    simplex solver took.

    This only makes sense if a problem was solved. See also
    the status instance variable.
*/
static VALUE lpsolve_time_total(VALUE self) ;
LPSOLVE_0_IN_TIME_OUT(time_total, timecreate, timeend);

/** A wrapper for solve_unscale().

    The get_bb_rule function returns the branch-and-bound rule for
    choosing which non-integer variable is to be selected. This rule
    can influence solving times considerably. Depending on the model
    one rule can be best and for another model another rule.  The
    default is NODE_PSEUDONONINTSELECT + NODE_GREEDYMODE +
    NODE_DYNAMICMODE + NODE_RCOSTFIXING (17445).

    @param self self
    @return  Returns the branch-and-bound rule.

*/
static VALUE lpsolve_unscale(VALUE self);
LPSOLVE_0_IN_STATUS_OUT(unscale)

/** A wrapper for lp_solve_version(). 
    @return 4-tuple: [major_version, minor_version, release, build]
*/
static VALUE
lpsolve_version(VALUE module_or_class) 
{
  int major_version;
  int minor_version;
  int release;
  int build;
  lp_solve_version(&major_version, &minor_version, &release, &build);
  return rb_ary_new3(4, 
		     INT2FIX(major_version), INT2FIX(minor_version),
		     INT2FIX(release), INT2FIX(build));
}

/** A wrapper for write_basis(). 

    The write_basis function writes the current basis to filename.
    This basis can later be reused by read_basis to reset a basis. Setting
    an initial basis can speed up the solver considerably. It is the
    starting point from where the algorithm continues to find an optimal
    solution.  When a restart is done, lp_solve continues at the last
    basis, except if set_basis, default_basis, guess_basis or read_basis
    is called.
    
    The basis in the file is written in MPS bas file format. 

    @return \a true if we could write the output file.
*/
static VALUE lpsolve_write_basis(VALUE self, VALUE filename);
LPSOLVE_1_STR_IN_BOOL_OUT(write_basis)

/** A wrapper for write_lp(). 

     The model in the file will be written in LP-format unless there
     is an error.  
     Note that row entry mode must be off, else this
     function also fails.  @see lpsolve_set_add_rowmode()

     @param self self 
     
     @param filename place to write LP file. If this is nil, then
     output is written the location specified by set_outputstream, or
     of that has not been set stdout.

     @return \a true if we could write the output file.
*/
static VALUE lpsolve_write_lp(int argc, VALUE *argv, VALUE self)
  {
    VALUE filename;
    unsigned int i_scanned;
    INIT_LP;
    if ((argc > 1) || (argc < 0))
      rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    i_scanned = rb_scan_args(argc, argv, "01", &filename);
    switch (i_scanned) {
    case 0: 
      RETURN_BOOL(write_lp(lp, NULL));
      break;
    case 1: 
      if (filename == Qnil)
	RETURN_BOOL(write_lp(lp, NULL));
      else if (TYPE(filename) != T_STRING) {
	report(lp, IMPORTANT, "%s: Parameter is not nil or a string filename.\n",
	       __FUNCTION__);
	return Qnil;
      } else
	RETURN_BOOL(write_lp(lp, RSTRING_PTR(filename)));
    default:
      rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", 
	       i_scanned);
    }
    return Qfalse;
  }

/** A wrapper for write_mps(). 

     The model in the file will be written in MPS-format unless there is an
     error.
     Note that row entry mode must be off, else this function also fails.
     @see lpsolve_set_add_rowmode()

     @param self self 
     
     @param filename place to write MPS file. If this is nil, then
     output is written the location specified by set_outputstream, or
     of that has not been set stdout.

     @return \a true if we could write the output file.
*/
static VALUE lpsolve_write_mps(int argc, VALUE *argv, VALUE self)
  {
    VALUE filename;
    unsigned int i_scanned;
    INIT_LP;
    if ((argc > 1) || (argc < 0))
      rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    i_scanned = rb_scan_args(argc, argv, "01", &filename);
    switch (i_scanned) {
    case 0: 
      RETURN_BOOL(write_mps(lp, NULL));
      
      break;
    case 1: 
      if (filename == Qnil)
	RETURN_BOOL(write_mps(lp, NULL));
      else if (TYPE(filename) != T_STRING) {
	report(lp, IMPORTANT, "%s: Parameter is not nil or a string filename.\n",
	       __FUNCTION__);
	return Qnil;
      } else
	RETURN_BOOL(write_mps(lp, RSTRING_PTR(filename)));
    default:
      rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", 
	       i_scanned);
    }
    return Qfalse;
  }

extern void init_lpsolve_constants();
/*#include "lpconsts.h" */

/** Called when we issue from Ruby: 
 \verbatim 
   require "lpsolve" 
 \endverbatim
*/
void Init_lpsolve()
{
  rb_cLPSolve = rb_define_class("LPSolve", rb_cObject);
  rb_define_alloc_func(rb_cLPSolve, lpsolve_alloc);

  init_lpsolve_constants();
  
  /* Class functions */
  rb_define_module_function(rb_cLPSolve, "make_lp",  lpsolve_make_lp, 2);
  rb_define_module_function(rb_cLPSolve, "read_LP",  lpsolve_read_LP, 3);
  rb_define_module_function(rb_cLPSolve, "read_MPS", lpsolve_read_MPS, 2);
  rb_define_module_function(rb_cLPSolve, "version",  lpsolve_version, 0);

  /* Class Methods */
  rb_define_method(rb_cLPSolve, "add_constraintex", 
		   lpsolve_add_constraintex, 4);
  rb_define_method(rb_cLPSolve, "add_SOS",          lpsolve_add_SOS, 4);
  rb_define_method(rb_cLPSolve, "default_basis",    lpsolve_default_basis, 0);
  rb_define_method(rb_cLPSolve, "del_column",       lpsolve_del_column, 1);
  rb_define_method(rb_cLPSolve, "del_constraint",   lpsolve_del_constraint, 1);
  rb_define_method(rb_cLPSolve, "get_bb_depthlimit",
		   lpsolve_get_bb_depthlimit, 0);
  rb_define_method(rb_cLPSolve, "get_bb_rule",      lpsolve_get_bb_rule, 0);
  rb_define_method(rb_cLPSolve, "get_col_name",     lpsolve_get_col_name, 1);
  rb_define_method(rb_cLPSolve, "get_col_num",      lpsolve_get_col_num, 1);
  rb_define_method(rb_cLPSolve, "get_column",       lpsolve_get_column, 1);
  rb_define_method(rb_cLPSolve, "get_infinite",     lpsolve_get_infinite, 0);
  rb_define_method(rb_cLPSolve, "get_lowbo",        lpsolve_get_lowbo, 1);
  rb_define_method(rb_cLPSolve, "get_lp_name",      lpsolve_get_lp_name, 0);
  rb_define_method(rb_cLPSolve, "get_mip_gap",      lpsolve_get_mip_gap, 1);
  rb_define_method(rb_cLPSolve, "get_Ncolumns",     lpsolve_get_Ncolumns, 0);
  rb_define_method(rb_cLPSolve, "get_Norig_columns",
		   lpsolve_get_Norig_columns, 0);
  rb_define_method(rb_cLPSolve, "get_Norig_rows",   lpsolve_get_Norig_rows, 0);
  rb_define_method(rb_cLPSolve, "get_Nrows",        lpsolve_get_Nrows, 0);
  rb_define_method(rb_cLPSolve, "get_nonzeros",     lpsolve_get_nonzeros, 0);
  rb_define_method(rb_cLPSolve, "get_mat",          lpsolve_get_mat, 2);
  rb_define_method(rb_cLPSolve, "get_objective",    lpsolve_get_objective, 0);
  rb_define_method(rb_cLPSolve, "get_origcol_name",
		   lpsolve_get_origcol_name, 1);
  rb_define_method(rb_cLPSolve, "get_origrow_name",
		   lpsolve_get_origrow_name, 1);
  rb_define_method(rb_cLPSolve, "get_presolve",     lpsolve_get_presolve, 0);
  rb_define_method(rb_cLPSolve, "get_presolveloops",
		   lpsolve_get_presolveloops, 0);
  rb_define_method(rb_cLPSolve, "get_row",          lpsolve_get_row, 1);
  rb_define_method(rb_cLPSolve, "get_row_name",     lpsolve_get_row_name, 1);
  rb_define_method(rb_cLPSolve, "get_scaling",      lpsolve_get_scaling, 0);
  rb_define_method(rb_cLPSolve, "get_simplextype",  lpsolve_get_simplextype, 0);
  rb_define_method(rb_cLPSolve, "get_solutioncount",
		   lpsolve_get_solutioncount, 0);
  rb_define_method(rb_cLPSolve, "get_solutionlimit",
		   lpsolve_get_solutionlimit, 0);
  rb_define_method(rb_cLPSolve, "get_status",       lpsolve_get_status, 0);
  rb_define_method(rb_cLPSolve, "get_statustext",   lpsolve_get_statustext, -1);
  rb_define_method(rb_cLPSolve, "get_timeout",      lpsolve_get_timeout, 0);
  rb_define_method(rb_cLPSolve, "get_total_iter",   lpsolve_get_total_iter, 0);
  rb_define_method(rb_cLPSolve, "get_upbo",         lpsolve_get_upbo, 1);
  rb_define_method(rb_cLPSolve, "get_var_dualresult", 
		   lpsolve_get_var_dualresult, 1);
  rb_define_method(rb_cLPSolve, "get_var_primalresult", 
		   lpsolve_get_var_primalresult, 1);
  rb_define_method(rb_cLPSolve, "get_variables",    lpsolve_get_variables, 0);
  rb_define_method(rb_cLPSolve, "get_verbose",      lpsolve_get_verbose, 0);
  rb_define_method(rb_cLPSolve, "initialize",       lpsolve_initialize, 2);
  rb_define_method(rb_cLPSolve, "is_debug",         lpsolve_is_debug, 0);
  rb_define_method(rb_cLPSolve, "is_maxim",         lpsolve_is_maxim, 0);
  rb_define_method(rb_cLPSolve, "is_SOS_var",       lpsolve_is_SOS_var, 1);
  rb_define_method(rb_cLPSolve, "presolve=",        lpsolve_set_presolve1, 1);
  rb_define_method(rb_cLPSolve, "print",            lpsolve_print, 0);
  rb_define_method(rb_cLPSolve, "print_debugdump",  
		   lpsolve_print_debugdump, 1);
  rb_define_method(rb_cLPSolve, "print_constraints",
		   lpsolve_print_constraints, 1);
  rb_define_method(rb_cLPSolve, "print_duals",      lpsolve_print_duals, 0);
  rb_define_method(rb_cLPSolve, "print_lp",         lpsolve_print_lp, 0);
  rb_define_method(rb_cLPSolve, "print_objective",  lpsolve_print_objective, 0);
  rb_define_method(rb_cLPSolve, "print_str",        lpsolve_print_str, 1);
  rb_define_method(rb_cLPSolve, "print_solution",   lpsolve_print_solution, 1);
  rb_define_method(rb_cLPSolve, "print_tableau",    lpsolve_print_tableau, 0);
  rb_define_method(rb_cLPSolve, "put_logfunc",      lpsolve_put_logfunc, 1);
  rb_define_method(rb_cLPSolve, "set_add_rowmode",  lpsolve_set_add_rowmode, 1);
  rb_define_method(rb_cLPSolve, "set_bb_depthlimit",
		   lpsolve_set_bb_depthlimit, 1);
  rb_define_method(rb_cLPSolve, "set_bb_rule",      lpsolve_set_bb_rule, 1);
  rb_define_method(rb_cLPSolve, "set_binary",       lpsolve_set_binary, 2);
  rb_define_method(rb_cLPSolve, "set_bounds",       lpsolve_set_bounds, 3);
  rb_define_method(rb_cLPSolve, "set_debug",        lpsolve_set_debug, 1);
  rb_define_method(rb_cLPSolve, "set_col_name",     lpsolve_set_col_name, 2);
  rb_define_method(rb_cLPSolve, "set_int",          lpsolve_set_int, 2);
  rb_define_method(rb_cLPSolve, "set_mat",          lpsolve_set_mat, 3);
  rb_define_method(rb_cLPSolve, "set_maxim",        lpsolve_set_maxim, 0);
  rb_define_method(rb_cLPSolve, "set_minim",        lpsolve_set_minim, 0);
  rb_define_method(rb_cLPSolve, "set_mip_gap",      lpsolve_set_mip_gap, 2);
  rb_define_method(rb_cLPSolve, "set_lowbo",        lpsolve_set_lowbo, 2);
  rb_define_method(rb_cLPSolve, "set_lp_name",      lpsolve_set_lp_name, 1);
  rb_define_method(rb_cLPSolve, "set_obj_fnex",     lpsolve_set_obj_fnex, 1);
  rb_define_method(rb_cLPSolve, "set_outputfile",   lpsolve_set_outputfile, 1);
  rb_define_method(rb_cLPSolve, "set_presolve",     lpsolve_set_presolve, 2);
  rb_define_method(rb_cLPSolve, "set_rh",           lpsolve_set_rh, 2);
  rb_define_method(rb_cLPSolve, "set_rh_range",     lpsolve_set_rh_range, 2);
  rb_define_method(rb_cLPSolve, "set_row_name",     lpsolve_set_row_name, 2);
  rb_define_method(rb_cLPSolve, "set_semicont",     lpsolve_set_semicont, 2);
  rb_define_method(rb_cLPSolve, "set_scaling",      lpsolve_set_scaling, 1);
  rb_define_method(rb_cLPSolve, "set_simplextype",  lpsolve_set_simplextype, 1);
  rb_define_method(rb_cLPSolve, "set_solutionlimit",
		   lpsolve_set_solutionlimit, 1);
  rb_define_method(rb_cLPSolve, "set_status",       lpsolve_set_status, 1);
  rb_define_method(rb_cLPSolve, "set_timeout",      lpsolve_set_timeout, 1);
  rb_define_method(rb_cLPSolve, "set_trace",        lpsolve_set_trace, 1);
  rb_define_method(rb_cLPSolve, "set_upbo",         lpsolve_set_upbo, 2);
  rb_define_method(rb_cLPSolve, "set_verbose",      lpsolve_set_verbose, 1);
  rb_define_method(rb_cLPSolve, "solve",            lpsolve_solve, 0);
  rb_define_method(rb_cLPSolve, "str_add_column",   lpsolve_str_add_column, 
		   1);
  rb_define_method(rb_cLPSolve, "str_add_constraint", 
		   lpsolve_str_add_constraint, 3);
  rb_define_method(rb_cLPSolve, "str_set_obj_fn", 
		   lpsolve_str_set_obj_fn, 1);
  rb_define_method(rb_cLPSolve, "time_elapsed",     lpsolve_time_elapsed, 0);
  rb_define_method(rb_cLPSolve, "time_load",        lpsolve_time_load, 0);
  rb_define_method(rb_cLPSolve, "time_presolve",    lpsolve_time_presolve, 0);
  rb_define_method(rb_cLPSolve, "time_simplex",     lpsolve_time_simplex, 0);
  rb_define_method(rb_cLPSolve, "time_total",       lpsolve_time_total, 0);
  rb_define_method(rb_cLPSolve, "unscale",          lpsolve_unscale, 0);
  rb_define_method(rb_cLPSolve, "version",          lpsolve_version, 0);
  rb_define_method(rb_cLPSolve, "write_basis",      lpsolve_write_basis, 1);
  rb_define_method(rb_cLPSolve, "write_lp",         lpsolve_write_lp, -1);
  rb_define_method(rb_cLPSolve, "write_mps",        lpsolve_write_mps, -1);

  /* Aliases accessors. */
  rb_define_alias(rb_cLPSolve, "bb_rule",        "get_bb_rule");
  rb_define_alias(rb_cLPSolve, "bb_rule=",       "set_bb_rule");
  rb_define_alias(rb_cLPSolve, "bb_depthlimit",  "get_bb_depthlimit");
  rb_define_alias(rb_cLPSolve, "bb_depthlimit=", "set_bb_depthlimit");
  rb_define_alias(rb_cLPSolve, "debug?",         "is_debug");
  rb_define_alias(rb_cLPSolve, "debug=",         "set_debug");
  rb_define_alias(rb_cLPSolve, "infinite",       "get_infinite");
  rb_define_alias(rb_cLPSolve, "maxim?",         "is_maxim");
  rb_define_alias(rb_cLPSolve, "lp_name",        "get_lp_name");
  rb_define_alias(rb_cLPSolve, "lp_name=",       "set_lp_name");
  rb_define_alias(rb_cLPSolve, "Ncolumns",       "get_Ncolumns");
  rb_define_alias(rb_cLPSolve, "Norig_columns",  "get_Norig_columns");
  rb_define_alias(rb_cLPSolve, "nonzeros",       "get_nonzeros");
  rb_define_alias(rb_cLPSolve, "Norig_rows",      "get_Norig_rows");
  rb_define_alias(rb_cLPSolve, "Nrows",          "get_Nrows");
  rb_define_alias(rb_cLPSolve, "objective",      "get_objective");
  rb_define_alias(rb_cLPSolve, "presolve",       "get_presolve");
  rb_define_alias(rb_cLPSolve, "presolveloops",  "get_presolveloops");
  /* rb_define_alias(rb_cLPSolve, "print",        "print_lp"); */
  rb_define_alias(rb_cLPSolve, "scaling",        "get_scaling");
  rb_define_alias(rb_cLPSolve, "scaling=",       "set_scaling");
  rb_define_alias(rb_cLPSolve, "status",         "get_status");
  rb_define_alias(rb_cLPSolve, "status=",        "set_status");
  rb_define_alias(rb_cLPSolve, "statustext",     "get_statustext");
  rb_define_alias(rb_cLPSolve, "simplextype",    "get_simplextype");
  rb_define_alias(rb_cLPSolve, "simplextype=",   "set_simplextype");
  rb_define_alias(rb_cLPSolve, "solutioncount",  "get_solutioncount");
  rb_define_alias(rb_cLPSolve, "solutionlimit",  "get_solutionlimit");
  rb_define_alias(rb_cLPSolve, "solutionlimit=", "set_solutionlimit");
  rb_define_alias(rb_cLPSolve, "sos_var?",       "is_SOS_var");
  rb_define_alias(rb_cLPSolve, "timeout",        "get_timeout");
  rb_define_alias(rb_cLPSolve, "timeout=",       "set_timeout");
  rb_define_alias(rb_cLPSolve, "total_iter",     "get_total_iter");
  rb_define_alias(rb_cLPSolve, "trace=",         "set_trace");
  rb_define_alias(rb_cLPSolve, "variables",      "get_variables");
  rb_define_alias(rb_cLPSolve, "verbose",        "get_verbose");
  rb_define_alias(rb_cLPSolve, "verbose=",       "set_verbose");
}


/* A revised version of print_lp or REPORT_lp. */
#include <lpsolve/lp_report.h>
void print(lprec *lp)
{
  int  i, j;
  unsigned int max_rowname = sizeof("lowbo")-1;
  char row_fmt[30];

  if(lp->outstream == NULL)
    return;

  if(lp->matA->is_roworder) {
    report(lp, IMPORTANT, "REPORT_lp: Cannot print lp while in row entry mode.\n");
    return;
  }

  fprintf(lp->outstream, "Model name: %s\n", get_lp_name(lp));

  for(j = 1; j <= lp->rows; j++) {
    unsigned int row_len = strlen(get_row_name(lp, j));
    if (max_rowname < row_len)
      max_rowname  = row_len;
  }

  memset(row_fmt, 0, sizeof(row_fmt));
  snprintf(row_fmt, sizeof(row_fmt), "%%-%ds ", max_rowname);
  fprintf(lp->outstream, row_fmt, " ");

  for(j = 1; j <= lp->columns; j++)
    fprintf(lp->outstream, "%8s ", get_col_name(lp,j));

  fprintf(lp->outstream, "\n%simize:\n", (is_maxim(lp) ? "Max" : "Min"));
  fprintf(lp->outstream, row_fmt, " ");

  for(j = 1; j <= lp->columns; j++) 
    {
      REAL val = get_mat(lp, 0, j);
      if (0.0 == val) 
	fprintf(lp->outstream, "%8s ", "");
      else {
	  fprintf(lp->outstream, "%8g ", get_mat(lp, 0, j));
      }
    }

  fprintf(lp->outstream, "\n\nSubject to:\n");

  for(i = 1; i <= lp->rows; i++) {
    fprintf(lp->outstream, row_fmt, get_row_name(lp, i));
    for(j = 1; j <= lp->columns; j++) 
      {
	REAL val = get_mat(lp, i, j);
	if (0.0 == val) 
	  fprintf(lp->outstream, "%8s ", "");
	else {
	  fprintf(lp->outstream, "%8g ", val);
	}
      }
    if(is_constr_type(lp, i, GE))
      fprintf(lp->outstream, ">= ");
    else if(is_constr_type(lp, i, LE))
      fprintf(lp->outstream, "<= ");
    else
      fprintf(lp->outstream, " = ");

    fprintf(lp->outstream, "%8g", get_rh(lp, i));

    if(is_constr_type(lp, i, GE)) {
      if(get_rh_upper(lp, i) < lp->infinite)
        fprintf(lp->outstream, "  %s = %8g", "upbo ", get_rh_upper(lp, i));
    }
    else if(is_constr_type(lp, i, LE)) {
      if(get_rh_lower(lp, i) > -lp->infinite)
        fprintf(lp->outstream, "  %s = %8g", "lowbo", get_rh_lower(lp, i));
    }
    fprintf(lp->outstream, "\n");
  }

  fprintf(lp->outstream, "\n");
  fprintf(lp->outstream, row_fmt, "Type");
  for(i = 1; i <= lp->columns; i++) {
    if(is_int(lp,i))
      fprintf(lp->outstream, "     Int ");
    else
      fprintf(lp->outstream, "    Real ");
  }

  fprintf(lp->outstream, "\n");
  fprintf(lp->outstream, row_fmt, "upbo ");
  for(i = 1; i <= lp->columns; i++)
    if(get_upbo(lp, i) >= lp->infinite)
      fprintf(lp->outstream, "     Inf ");
    else
      fprintf(lp->outstream, "%8g ", get_upbo(lp, i));
  fprintf(lp->outstream, "\n");
  fprintf(lp->outstream, row_fmt, "lowbo");
  for(i = 1; i <= lp->columns; i++)
    if(get_lowbo(lp, i) <= -lp->infinite)
      fprintf(lp->outstream, "    -Inf ");
    else
      fprintf(lp->outstream, "%8g ", get_lowbo(lp, i));
  fprintf(lp->outstream, "\n");
  fprintf(lp->outstream, row_fmt, "SOS  ");
  for(i = 1; i <= lp->columns; i++) {
    if(is_SOS_var(lp, i)) {
      fprintf(lp->outstream, "    true ");
    } else
      fprintf(lp->outstream, "   false ");
  }
  fprintf(lp->outstream, "\n");
  if (lp->SOS) {
    SOSgroup *SOS_group = lp->SOS;
    SOSrec **sos_list;
    sos_list = SOS_group->sos_list;
    fprintf(lp->outstream, "\nSOS groups:\n");
    for (i=0; i<lp->SOS->sos_count; i++) {
      unsigned int j;
      fprintf(lp->outstream, "\t%s: Type: %d, with %d members",
	      sos_list[i]->name, sos_list[i]->type, sos_list[i]->size);
      for(j = 1; j <= sos_list[i]->size; j++) {
	int i_col_num = sos_list[i]->members[j];
	char *psz_col_name;
	if (i_col_num <= lp->columns) {
	  psz_col_name = get_col_name(lp, i_col_num);
	  if (NULL != psz_col_name)
	    fprintf(lp->outstream, "%s%s", (j > 1) ? ", " : ": ",
		    psz_col_name);
	}
      }
      fprintf(lp->outstream, "\n");
    }
  }
  fflush(lp->outstream);
}
