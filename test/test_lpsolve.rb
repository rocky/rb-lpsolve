#!/usr/bin/env ruby
# $Id: test_lpsolve.rb,v 1.13 2007/03/28 11:48:01 rocky Exp $
require 'test/unit'
require 'rubygems'

# require 'ruby-debug' ; Debugger.start

Mypath  = File.expand_path(File.dirname(__FILE__))
old_dir = File.expand_path(Dir.pwd)
if old_dir != Mypath
  Dir.chdir(Mypath)
end
require Mypath + '/../ext/lpsolve.so'

def compare_files(file_correct, file_check_against, max_line=0, filter=nil)
  check_against = File.new(file_check_against)
  got_lines = check_against.readlines
  correct = File.new(file_correct)
  correct_lines = correct.readlines
  filter.call(got_lines, correct_lines) if filter

  correct_lines.each_with_index do |line, lineno|
    begin
      break if max_line > 0 && lineno >= max_line - 1
      if got_lines[lineno] != correct_lines[lineno]
        puts "At line #{lineno}, we expected:"
        puts correct_lines[lineno]
        puts 'but got:'
        puts got_lines[lineno]
        return false
      end
    rescue EOFError
      return false
    end
  end
  if correct_lines.size != got_lines.size
    puts('difference in number of lines: ' + 
         "#{correct_lines.size} vs. #{got_lines.size}")
    return false
  end
  return true
end

class TestLPSolve < Test::Unit::TestCase
  def setup
    @lp = LPSolve.new(0, 4)
  end

  def test_basic
    assert_equal(Object, LPSolve.superclass)
    assert_equal(LPSolve, @lp.class)

    assert_equal(4, @lp.version.length)
    @lp.print_lp()

    [
     # Constraint relations
     [0, 'FR'], [1, 'LE'], [2, 'GE'], [3, 'EQ'],
     
     # Verbosity constant
     [0, 'NEUTRAL'],   [1, 'CRITICAL'], [2, 'SEVERE'], 
     [3, 'IMPORTANT'], [4, 'NORMAL'],   [5, 'DETAILED'], 
     [6, 'FULL'],       
     
     # Scaling constants
     
     [0, 'SCALE_NONE'], [1, 'SCALE_EXTREME'], [2, 'SCALE_RANGE'],
     [3, 'SCALE_MEAN'], [4, 'SCALE_GEOMETRIC'], 
     [7, 'SCALE_CURTISREID'],
     
     # Simplex types
     [ 5, 'SIMPLEX_PRIMAL_PRIMAL'],
     [ 6, 'SIMPLEX_DUAL_PRIMAL'],
     [ 9, 'SIMPLEX_PRIMAL_DUAL'],
     [10, 'SIMPLEX_DUAL_DUAL'],
     
     # Solve types
     [-2, 'NOMEMORY'],   [ 0, 'OPTIMAL'],   [ 1, 'SUBOPTIMAL'],
     [ 2, 'INFEASIBLE'], [ 3, 'UNBOUNDED'], [ 4, 'DEGENERATE'],
     [ 5, 'NUMFAILURE'], [ 6, 'USERABORT'], [ 7, 'TIMEOUT'],
     [ 9, 'PRESOLVED'],  [10, 'PROCFAIL'],  [11, 'PROCBREAK'],
     [12, 'FEASFOUND'],  [13, 'NOFEASFOUND']
    ].each do |val, sym|
        assert_equal(val, eval('LPSolve::' + sym))
    end
    
    # Brand-and-bound rules
    [
     [   0, 'NODE_FIRSTSELECT'], 
     [   1, 'NODE_GAPSELECT'], 
     [   2, 'NODE_RANGESELECT'], 
     [   3, 'NODE_FRACTIONSELECT'], 
     [   4, 'NODE_PSEUDOCOSTSELECT'], 
     [   5, 'NODE_PSEUDONONINTSELECT'], 
     [   6, 'NODE_PSEUDORATIOSELECT'], 
     [   7, 'NODE_USERSELECT'], 
     [   8, 'NODE_WEIGHTREVERSEMODE'], 
     [  16, 'NODE_BRANCHREVERSEMODE'], 
     [  32, 'NODE_GREEDYMODE'], 
     [  64, 'NODE_PSEUDOCOSTMODE'], 
     [ 128, 'NODE_DEPTHFIRSTMODE'], 
     [ 256, 'NODE_RANDOMIZEMODE'], 
     [1024, 'NODE_DYNAMICMODE'], 
     [2048, 'NODE_RESTARTMODE'], 
     [4096, 'NODE_BREADTHFIRSTMODE'], 
     [8192, 'NODE_AUTOORDER']
    ].each do |val, sym|
      assert_equal(val, eval('LPSolve::' + sym))
      @lp.bb_rule = val
      assert_equal(val, @lp.bb_rule)
    end
  end
  
  def test_constraint
    @lp.set_outputfile('lpsolve.out')
    @lp.print_str("print_str() test\n")
    assert(@lp.str_add_constraint("3 2 2 1", LPSolve::LE, 4),
           "Error in adding <= constraint")
    @lp.print_lp()
    assert(@lp.str_add_constraint("0 4 3 1", LPSolve::GE, 3),
           "Error in adding >= constraint")
    @lp.print_lp()
    assert true
    puts "Setting the objective function: 2 3 -2 3"
    assert(@lp.str_set_obj_fn("2 3 -2 3"),
           "Error in setting objective function")
    @lp.verbose = LPSolve::IMPORTANT
    @lp.print()
    solution = @lp.solve
    assert_equal(0, solution)
    assert_equal(0, @lp.status)
    assert_equal("OPTIMAL solution", @lp.get_statustext(@lp.status))
    @lp.lp_name = "Test model"
    @lp.set_col_name(1, "Column 1")
    @lp.set_row_name(1, "Row 1")
    @lp.print_objective
    @lp.print_solution(1)
    @lp.print_solution(-1)
    @lp.print_constraints(1)
    @lp.print_duals()
    assert @lp.set_mat(2,1,0.5)
    assert_equal(2.0 ,   @lp.get_mat(1, 2))
    assert_equal(-4.0 ,  @lp.objective)
    assert_equal(-4.0 ,  @lp.get_var_primalresult(0))
    assert_equal(4.0,    @lp.get_var_primalresult(1))
    assert_in_delta(6.0, @lp.get_var_primalresult(2), 0.001)
    assert_in_delta(5.0, @lp.get_var_dualresult(3),   0.001)
    assert_in_delta(5.0, @lp.get_var_dualresult(4),   0.001)
    assert_equal(0.0,    @lp.get_var_dualresult(5))
    assert_equal(0,      @lp.get_var_primalresult(3))
    assert_equal(0,      @lp.get_var_primalresult(4))
    assert_equal(2.0,    @lp.get_var_primalresult(5))
    assert_equal(1,      @lp.get_solutioncount)
    assert_equal(1,      @lp.solutioncount)
    @lp = nil
    filter = Proc.new{|got_lines, correct_lines|
      [got_lines[49]].flatten.each do |s|
        s.sub!(/-1e[+]30\s+.*\s+-1e[+]30/, "-1e+30\t\t0.0\t\t-1e+30")
      end
    }
    assert compare_files('lpsolve.right', 'lpsolve.out', 0, filter)
  end

  # Check set_col_name(), get_col_name(), and get_origcol_name()
  def test_col_name
    # Test Invalid parameter type, should be a string.
    assert_equal(nil, @lp.set_col_name(1, true))

    col_name="column test name"
    assert_equal(nil,  @lp.set_col_name([1], col_name), 
                 "Column name must be an int")
    assert_equal(nil,  @lp.set_col_name(1, [1]), 
                 "Column name must be an int")
    assert @lp.set_col_name(1, col_name)
    assert_equal(col_name, @lp.get_col_name(1))
    assert_equal(col_name, @lp.get_origcol_name(1))
    assert_equal(nil, @lp.get_col_name(100))
    assert_equal(nil, @lp.get_col_name("A"))
    assert_equal(1, @lp.get_col_num(col_name))
  end

  # Check get_Nrows(), get_Ncolumns(), get_Norig_rows(), get_Norig_columns()
  # Check reading an MPS and the solution.
  def test_Ncols_rows
    # Test Invalid parameter type, should be a string.
    assert_equal(4, @lp.Ncolumns)
    assert_equal(4, @lp.Norig_columns)
    assert_equal(0, @lp.Nrows)
    assert_equal(0, @lp.Norig_rows)
    @lp = LPSolve.read_MPS("../example/model.mps", LPSolve::NORMAL)
    assert_equal(nil, @lp.add_SOS("bad", 1, 1, []))
    presolve =  LPSolve::PRESOLVE_COLS + 
      LPSolve::PRESOLVE_LINDEP +
      LPSolve::PRESOLVE_SOS +
      LPSolve::PRESOLVE_REDUCEMIP +
      LPSolve::PRESOLVE_KNAPSACK 
    @lp.presolve = [presolve, presolve]
    @lp.solve()
    assert_equal(0, @lp.status)
    assert_equal(2, @lp.Ncolumns)
    assert_equal(2, @lp.Norig_columns)
    assert_equal(3, @lp.Nrows)
    assert_equal(3, @lp.Norig_rows)
    assert_equal(0.0, @lp.get_objective())
    assert_equal(1, @lp.solutioncount())
  end

  # Check set_col_name(), get_col_name(), and get_origcol_name()
  def test_nonzeros
    # Test Invalid parameter type, should be a string.
    assert_equal(0, @lp.nonzeros, 
                 "Initially there are 0 nonzero entries - everything is zero")
    assert_equal(0, @lp.get_nonzeros, 
                 "Initially there are 0 nonzero entries - everything is zero")
    assert(@lp.str_add_constraint("0 4 3 1", LPSolve::GE, 3),
           "Error in adding <= constraint")
    assert_equal(3, @lp.nonzeros, 
                 "We just set 3 entries to be non-zero")
  end

  # Check get_Nrows(), get_Ncolumns(), get_Norig_rows(), get_Norig_columns()
  # Check set/get_timeout
  def test_get_timeout
    assert_equal Float, @lp.get_timeout().class
    @lp.set_timeout 10.0
    assert_equal(10.0, @lp.get_timeout())
  end

  # Check get_bb_depthlimit set_bb_depthlimit
  def test_get_bb_depthlimit
    @lp.bb_depthlimit = 10
    assert_equal(10, @lp.bb_depthlimit)
    @lp.set_bb_depthlimit(-5)
    assert_equal(-5, @lp.get_bb_depthlimit)
  end

  # Check get_bb_depthlimit set_bb_depthlimit
  def test_set_mip_gap
    @lp.set_mip_gap(true, 10)
    assert_equal(10, @lp.get_mip_gap(true))
    @lp.set_mip_gap(false, 1.0)
    assert_equal(1.0, @lp.get_mip_gap(false))
  end

  # Check set_*variable* types
  def test_vartype
    [:set_binary, :set_int, :set_semicont].each do |fn|
      assert(@lp.send(fn, 1, true),
              "Should have been able to set variable type (#{fn}) for " +
             "column 1")

      assert(@lp.send(fn, 1, false),
              "Should have been able to unset variable type (#{fn}) for " +
             "column 1")

      assert(@lp.send(fn, 1, nil),
              "Should have been able to unset variable type (#{fn}) via " +
             "nil for column 1")

      assert(!@lp.send(fn, 100, false),
              "Should not have been able to set variable type (#{fn}) " +
             "since column number is too large")

      assert(!@lp.send(fn, 1, "not a boolean parameter"),
              "Should not gotten an error on (#{fn}) " +
             "because parameter 2 is not boolean or nil")
    end
  end

  # Check set_debug() and is_debug() and accessor functions to them.
  def test_debug
    assert_equal(false, eval("@lp.set_debug(5)"))  # bad value
    assert_equal(false, eval("@lp.set_debug('true')"))  # bad value
    assert eval("@lp.debug = true")
    assert @lp.is_debug
    assert @lp.debug?
    @lp.set_debug(false)
    assert_equal(@lp.is_debug, false)
  end

  # Check our deallocation routine doesn't mess
  # things up across a garbage collection
  def test_gc
    @lp = LPSolve.new(0, 4)
    GC.enable
    GC.start
    assert_equal(LPSolve, @lp.class)
  end

  # Check set_row_name(), set_orig_row_name() and get_row_name()
  # and corresponding accessor functions
  def test_lp_name
    assert_equal(String, @lp.lp_name.class) # accessor function
    assert @lp.set_lp_name("Test model")
    assert_equal("Test model", @lp.get_lp_name())
    @lp.lp_name = "Test 2"
    assert_equal("Test 2", @lp.lp_name())
  end

  # Check set_mat(), get_mat(), set_add_rowmode(), str_add_constraint()
  def test_mat
    assert_equal(nil , @lp.get_mat(1.01, 2), "Row number should be int")
    assert_equal(nil , @lp.get_mat(1, "a"), "Col number should be int")
    assert_equal(0.0 , @lp.get_mat(1, 2))
    assert_equal(nil,  @lp.set_add_rowmode(1), "Parameter should be a boolean")
    assert_equal(true, @lp.set_add_rowmode(true), 
                 "The default should have been false")
    assert_equal(false, @lp.set_add_rowmode(true), 
                 "We should have just set rowmode to true")
    assert_equal(nil, @lp.add_constraintex(nil, [], 
                                         LPSolve::LE, 3),
                 "add_constraintex")
    assert_equal(nil, @lp.add_constraintex(nil, 
                                         [[1, 3], [2, 2], ["a", 2], [4, 1]], 
                                         LPSolve::LE, 3),
                 "add_constraintex")
    assert_equal(1, @lp.add_constraintex("Row1", 
                                         [[1, 3], [2, 2], [3, 2], [4, 1]], 
                                         LPSolve::LE, 4),
                 "add_constraintex")
    assert(@lp.str_add_constraint("0 4 3 1", LPSolve::GE, 3),
           "Error in adding <= constraint")
    assert_equal(nil, @lp.set_obj_fnex("bad"))
    assert(@lp.set_obj_fnex([[1, 2], [2, 3], [3, -2], [4, 3]]),
           "Error in setting objective function via set_obj_fnex")
    assert(@lp.set_add_rowmode(false), 
           "Rowmode should have been set true and is now false")
    assert @lp.set_mat(2,1,0.5)
    assert_equal(2.0 , @lp.get_mat(1, 2))
  end

  # Check our deallocation routine doesn't mess
  # things up across a garbage collection
  def test_putlogfunc
    @lp.put_logfunc("puts")
  end

  # Check read_LP()
  def test_read_LP
    # Check invalid parameter type. Should be a string.
    assert_equal(nil, LPSolve.read_LP(TRUE, LPSolve::IMPORTANT, "LP model"))
    assert_equal(nil, LPSolve.read_LP("model.lp", LPSolve::IMPORTANT, 5.0))

    lp = LPSolve.read_LP("../example/model.lp", LPSolve::IMPORTANT, "LP model")
    assert_equal(LPSolve, lp.class)
    lp.set_outputfile("lp_model.out")
    lp.set_verbose(LPSolve::NORMAL)
    lp.solve
    
    assert_in_delta(lp.time_total, 
                    lp.time_load + lp.time_presolve + lp.time_simplex,
                    0.01)
    assert(lp.time_total >= lp.time_elapsed, 
           ("total time %g should be larger than elapsed time %g" % 
           [lp.time_total, lp.time_elapsed]))

    filter = Proc.new{|got_lines, correct_lines|
      [got_lines[49]].flatten.each do |s|
        s.sub!(/-1e[+]30\s+.*\s+-1e[+]30/, "-1e+30\t\t0.0\t\t-1e+30") if s
      end
    }
    assert compare_files("lp_model.right", "lp_model.out", 23, filter)
    assert_in_delta(21.875, lp.variables[0], 0.0001)
    assert_in_delta(53.125, lp.variables[1], 0.0001)
    assert_equal([143.0, 120.0, 110.0, 1.0], lp.get_column(1))
    assert_equal(nil, lp.get_column(0))
    assert_in_delta(60.0, lp.get_column(2)[0], 0.0001)
    assert_in_delta(210.0, lp.get_column(2)[1], 0.0001)
    assert_equal(0, lp.get_lowbo(1))
    assert_equal(lp.infinite, lp.get_upbo(1))
    assert(lp.set_bounds(1, -1, 200.1))
    assert_equal(false, lp.set_bounds(0, -1, 200.1))
    assert_equal(200.1, lp.get_upbo(1))
    assert_equal(-1, lp.get_lowbo(1))
    assert lp.maxim?
  end

  # Check read_MPS()
  def test_read_MPS
    # Check invalid parameter type. Should be a string.
    assert_equal(nil, LPSolve.read_MPS(5, LPSolve::IMPORTANT))
    
    lp = LPSolve.read_MPS("../example/model.mps", LPSolve::IMPORTANT)
    assert_equal(LPSolve, lp.class)
    lp.set_outputfile("mps_model.out")
    lp.set_verbose(LPSolve::NORMAL)
    lp.set_lp_name("MPS model")
    lp.solve
    assert compare_files("mps_model.right", "mps_model.out", 23)
    assert_in_delta(0.0, lp.variables[0], 0.0001)
    assert_in_delta(0.0, lp.variables[1], 0.0001)
    assert_equal([143.0, 120.0, 110.0, 1.0], lp.get_column(1))
    assert_equal(nil, lp.get_column(0))
    assert_in_delta(60.0, lp.get_column(2)[0], 0.0001)
    assert_in_delta(210.0, lp.get_column(2)[1], 0.0001)
    assert_equal(0, lp.get_lowbo(1))
    assert(lp.set_lowbo(1, 1))
    assert_equal(1, lp.get_lowbo(1))
    assert_equal(lp.infinite, lp.get_upbo(1))
    
    assert(lp.set_upbo(1, 100))
    assert_equal(100, lp.get_upbo(1))
    assert(lp.write_lp("foo.lp"))
    assert(lp.write_mps("foo.mps"))
    ## FIXME
    # assert(lp.write_mps(nil))
    # assert(lp.write_mps())
  end

  # Check set_row_name(), set_orig_row_name() and get_row_name()
  def test_row_name
    # Test Invalid parameter type, should be a string.
    assert_equal(false, @lp.set_row_name(1, true))

    assert @lp.set_row_name(1, "row test name")
    assert_equal("row test name", @lp.get_row_name(1))
    assert_equal("row test name", @lp.get_origrow_name(1))
    assert_equal(nil, @lp.get_origrow_name([1]))
    assert_equal(@lp.get_row_name(100), nil)
    assert_equal(@lp.get_row_name("a"), nil)
  end

  # Check set_rh() and set_rh_range
  def test_set_rh
    # Test Invalid parameter type, should be a string.
    assert(@lp.str_set_obj_fn("1 0 0 0"),
           "Error in setting objective function via set_obj_fnex")
    assert_equal(nil, @lp.set_rh(0, 6.0),
                 "Should get false on set_rh because row hasn't been set yet")
    assert_equal(false, @lp.set_rh_range(1, 2.0),
                 "Should get false on _set_rh - row not set yet")
    @lp.add_constraintex("ROW1", [[1, 1.0]], LPSolve::GE, 5.0)
    @lp.set_rh(1, 6.0) # Reset above range from 5 to 6.
    assert @lp.set_rh_range(1, 2.0) # Set lower bound to 6.0 - 2.0
    assert(@lp.set_maxim, "Should have been able to maximize")
    assert_equal(0, @lp.solve())
    # FIXME: test solution
    assert(@lp.set_minim, "Should have been able to minimmize")
    assert_equal(0, @lp.solve())
    # FIXME: test solution
  end

  # Check set_scaling(), get_scaling()
  def test_scaling
    scalemodes = [LPSolve::SCALE_NONE,
                  LPSolve::SCALE_EXTREME,
                  LPSolve::SCALE_RANGE,
                  LPSolve::SCALE_MEAN,
                  LPSolve::SCALE_GEOMETRIC,
                  LPSolve::SCALE_CURTISREID]
    for scalemode in scalemodes do
      @lp.set_scaling(scalemode)
      assert_equal(scalemode, @lp.get_scaling())
    end
  end

  # Check set_simplextype(), get_simplextype()
  def test_simplextype
    simplextypes = [
                  LPSolve::SIMPLEX_PRIMAL_PRIMAL, 
                  LPSolve::SIMPLEX_DUAL_PRIMAL, 
                  LPSolve::SIMPLEX_PRIMAL_DUAL, 
                  LPSolve::SIMPLEX_DUAL_DUAL
                ]
    assert_equal LPSolve::SIMPLEX_DUAL_PRIMAL, @lp.simplextype
    for simplextype in simplextypes do
      @lp.set_simplextype(simplextype)
      assert_equal(simplextype, @lp.get_simplextype())
    end
    for simplextype in simplextypes do
      @lp.simplextype = simplextype
      assert_equal(simplextype, @lp.simplextype)
    end
  end

  # Check set_debug() and is_debug()
  def test_verbose
    assert_equal(Fixnum, @lp.get_verbose.class)
    @lp.set_verbose(LPSolve::CRITICAL)
    assert_equal LPSolve::CRITICAL, @lp.verbose
    @lp.verbose = LPSolve::NORMAL
  end

  def test_solutioncount_limit
    @lp = LPSolve.new(0, 4)
    assert_equal(0, @lp.solutioncount,
                 "Haven't solved anything yet so solutioncount should be 0.")
    assert_equal(1, @lp.solutionlimit,
                 "The default solution limit should be 1.")
    assert_equal(17445, @lp.bb_rule,
                 "This is the default bb_rule mask. Have defaults changed? ")
    @lp.solutionlimit = 3
    assert_equal(3, @lp.get_solutionlimit,
                 "We just previously set the solution limit to 3.")
    @lp.set_lp_name("Four equal solutions")
    @lp.set_add_rowmode(true)
    
    # Maximize:
    @lp.set_obj_fnex([[1, 100], [2, 100], [3, 100], [4, 100]])
    
    @lp.add_constraintex("Only two", [[1, 1], [2, 1], [3, 1], [4, 1]], 
                         LPSolve::LE, 2)
    @lp.add_SOS("SOS 1 or 2", 1, 1, [[1, 0], [2, 1]])
    @lp.add_SOS("SOS 3 or 4", 1, 1, [[3, 0], [4, 1]])
    @lp.set_binary(1, true)
    @lp.set_binary(2, true)
    @lp.set_binary(3, true)
    @lp.set_binary(4, true)
    @lp.set_add_rowmode(false)
    assert(@lp.set_maxim, "Should have been able to maximize")
    assert_equal(0, @lp.solve)
  end

  def test_status
    @lp = LPSolve.new(0, 1)
    assert_equal("LPSolve method solve() not performed yet.", 
                 @lp.get_statustext)
    assert_equal(0, @lp.solve())
    assert_equal("OPTIMAL solution", @lp.get_statustext)
    assert @lp.str_add_constraint("1", LPSolve::EQ, 4)
    assert @lp.str_add_constraint("2", LPSolve::EQ, 2)
    assert_equal(2, @lp.solve())
    assert_equal(2, @lp.status)
    assert_equal("Model is primal INFEASIBLE", @lp.statustext)
  end

end
