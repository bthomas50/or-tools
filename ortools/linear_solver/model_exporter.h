// Copyright 2010-2018 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_LINEAR_SOLVER_MODEL_EXPORTER_H_
#define OR_TOOLS_LINEAR_SOLVER_MODEL_EXPORTER_H_

#include <string>
#include <vector>

#include "ortools/base/hash.h"
#include "ortools/base/macros.h"

namespace operations_research {

class MPConstraint;
class MPObjective;
class MPVariable;

class MPModelProto;

class MPModelProtoExporter {
 public:
  // The argument must live as long as this class is active.
  explicit MPModelProtoExporter(const MPModelProto& proto);

  // Outputs the current model (variables, constraints, objective) as a
  // std::string encoded in the so-called "CPLEX LP file format" as generated by
  // SCIP. The LP file format is easily readable by a human.
  //
  // Returns false if some error has occurred during execution.
  // The validity of names is automatically checked. If a variable name or a
  // constraint name is invalid or non-existent, a new valid name is
  // automatically generated.
  //
  // If 'obfuscated' is true, the variable and constraint names of proto_
  // are not used.  Variable and constraint names of the form "V12345"
  // and "C12345" are used instead.
  //
  // For more information about the different LP file formats:
  // http://lpsolve.sourceforge.net/5.5/lp-format.htm
  // The following give a reasonable idea of the CPLEX LP file format:
  // http://lpsolve.sourceforge.net/5.5/CPLEX-format.htm
  // http://tinyurl.com/cplex-lp-format
  // http://www.gurobi.com/documentation/5.1/reference-manual/node871
  bool ExportModelAsLpFormat(bool obfuscated, std::string* output);

  // Outputs the current model (variables, constraints, objective) as a
  // std::string encoded in MPS file format, using the "fixed" MPS format if
  // possible, and the "free" MPS format otherwise.
  //
  // Returns false if some error has occurred during execution. Models with
  // maximization objectives trigger an error, because MPS can encode only
  // minimization problems.
  //
  // If fixed_format is true, the method tries to use the MPS fixed format (the
  // use of which is discouraged as coefficients are printed with less
  // precision). If it is not possible to use the fixed format, the method falls
  // back to the so-called "free format".
  //
  // The validity of names is automatically checked. If a variable name or a
  // constraint name is invalid or non-existent, a new valid name is
  // automatically generated.
  //
  // Name validity and obfuscation works exactly as in ExportModelAsLpFormat().
  //
  // For more information about the MPS format:
  // http://en.wikipedia.org/wiki/MPS_(format)
  // A close-to-original description coming from OSL:
  // http://tinyurl.com/mps-format-by-osl
  // A recent description from CPLEX:
  // http://tinyurl.com/mps-format-by-cplex
  // CPLEX extensions:
  // http://tinyurl.com/mps-extensions-by-cplex
  // Gurobi's description:
  // http://www.gurobi.com/documentation/5.1/reference-manual/node869
  bool ExportModelAsMpsFormat(bool fixed_format, bool obfuscated,
                              std::string* output);

 private:
  // Computes the number of continuous, integer and binary variables.
  // Called by ExportModelAsLpFormat() and ExportModelAsMpsFormat().
  void Setup();

  // Processes all the proto.name() fields and returns the result in a vector.
  //
  // If 'obfuscate' is true, none of names are actually used, and this just
  // returns a vector of 'prefix' + proto index (1-based).
  //
  // If it is false, this tries to keep the original names, but:
  // - if the first character is forbidden, '_' is added at the beginning of
  //   name.
  // - all the other forbidden characters are replaced by '_'.
  // To avoid name conflicts, a '_' followed by an integer is appended to the
  // result.
  //
  // If a name is longer than the maximum allowed name length, the obfuscated
  // name is used.
  //
  // This method also sets use_fixed_mps_format_ to false if one name is too
  // long.
  //
  // Therefore, a name "$20<=40" for proto #3 could be "_$20__40_1".
  template <class ListOfProtosWithNameFields>
  std::vector<std::string> ExtractAndProcessNames(
      const ListOfProtosWithNameFields& proto, const std::string& prefix,
      bool obfuscate);

  // Returns true when the fixed MPS format can be used.
  // The fixed format is used when the variable and constraint names do not
  // exceed 8 characters. In the case of an obfuscated file, this means that
  // the maximum number of digits for constraints and variables is limited to 7.
  bool CanUseFixedMpsFormat() const;

  // Appends a general "Comment" section with useful metadata about the model
  // to "output".
  // Note(user): there may be less variables in output than in the original
  // model, as unused variables are not shown by default. Similarly, there
  // may be more constraints in a .lp file as in the original model as
  // a constraint lhs <= term <= rhs will be output as the two constraints
  // term >= lhs and term <= rhs.
  void AppendComments(const std::string& separator, std::string* output) const;

  // Clears "output" and writes a term to it, in "Lp" format. Returns false on
  // error (for example, var_index is out of range).
  bool WriteLpTerm(int var_index, double coefficient,
                   std::string* output) const;

  // Appends a pair name, value to "output", formatted to comply with the MPS
  // standard.
  void AppendMpsPair(const std::string& name, double value,
                     std::string* output) const;

  // Appends the head of a line, consisting of an id and a name to output.
  void AppendMpsLineHeader(const std::string& id, const std::string& name,
                           std::string* output) const;

  // Same as AppendMpsLineHeader. Appends an extra new-line at the end the
  // std::string pointed to by output.
  void AppendMpsLineHeaderWithNewLine(const std::string& id,
                                      const std::string& name,
                                      std::string* output) const;

  // Appends an MPS term in various contexts. The term consists of a head name,
  // a name, and a value. If the line is not empty, then only the pair
  // (name, value) is appended. The number of columns, limited to 2 by the MPS
  // format is also taken care of.
  void AppendMpsTermWithContext(const std::string& head_name,
                                const std::string& name, double value,
                                std::string* output);

  // Appends a new-line if two columns are already present on the MPS line.
  // Used by and in complement to AppendMpsTermWithContext.
  void AppendNewLineIfTwoColumns(std::string* output);

  // When 'integrality' is true, appends columns corresponding to integer
  // variables. Appends the columns for non-integer variables otherwise.
  // The sparse matrix must be passed as a vector of columns ('transpose').
  void AppendMpsColumns(
      bool integrality,
      const std::vector<std::vector<std::pair<int, double>>>& transpose,
      std::string* output);

  // Appends a line describing the bound of a variablenew-line if two columns
  // are already present on the MPS line.
  // Used by and in complement to AppendMpsTermWithContext.
  void AppendMpsBound(const std::string& bound_type, const std::string& name,
                      double value, std::string* output) const;

  const MPModelProto& proto_;

  // Vector of variable names as they will be exported.
  std::vector<std::string> exported_variable_names_;

  // Vector of constraint names as they will be exported.
  std::vector<std::string> exported_constraint_names_;

  // Number of integer variables in proto_.
  int num_integer_variables_;

  // Number of binary variables in proto_.
  int num_binary_variables_;

  // Number of continuous variables in proto_.
  int num_continuous_variables_;

  // Current MPS file column number.
  int current_mps_column_;

  // True is the fixed MPS format shall be used.
  bool use_fixed_mps_format_;

  // True if the variable and constraint names will be obfuscated.
  bool use_obfuscated_names_;

  DISALLOW_COPY_AND_ASSIGN(MPModelProtoExporter);
};

}  // namespace operations_research

#endif  // OR_TOOLS_LINEAR_SOLVER_MODEL_EXPORTER_H_
