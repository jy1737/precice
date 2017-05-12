#pragma once

#include <boost/test/unit_test.hpp>
#include "utils/Parallel.hpp"
#include "math/math.hpp"

#ifndef PRECICE_NO_PETSC
#include "petscsys.h"
#endif

namespace precice { namespace testing {

namespace bt = boost::unit_test;
using Par = precice::utils::Parallel;

/// Fixture to set and reset MPI communicator
struct MPICommRestrictFixture {
  explicit MPICommRestrictFixture(std::vector<int> & ranks)
  {
    // Restriction MUST always be called on all ranks, otherwise we hang
    if (static_cast<int>(ranks.size()) < Par::getCommunicatorSize()) {
      Par::setGlobalCommunicator(Par::getRestrictedCommunicator(ranks));
    }

    // if we set always set PETSC_COMM_WORLD to the restricted communicator, PETSc crashes
    #ifndef PRECICE_NO_PETSC
    if (ranks.size() == 1)
      PETSC_COMM_WORLD = PETSC_COMM_SELF;
    #endif
  }
  
  ~MPICommRestrictFixture()
  {
    Par::setGlobalCommunicator(Par::getCommunicatorWorld());
    #ifndef PRECICE_NO_PETSC
    PETSC_COMM_WORLD = MPI_COMM_WORLD;
    #endif
  }
};


/// Boost.Test decorator that makes the test run only on specfic ranks.
/*
 * This does not restrict the communicator, which must be done by installing the MPICommrestrictFixture.
 */
class OnRanks : public bt::decorator::base
{
public:

  explicit OnRanks(const std::vector<int> & ranks) :
    _ranks(ranks)
  {}

private:
  
  virtual void apply(bt::test_unit& tu)
  {
    size_t myRank = Par::getProcessRank();
    size_t size = Par::getCommunicatorSize();
    
    // If current rank is not in requested ranks
    if (std::find(_ranks.begin(), _ranks.end(), myRank) == _ranks.end()) {
      bt::framework::get<bt::test_suite>(tu.p_parent_id).remove(tu.p_id);
      return;
    }

    // If more ranks requested than available
    if (_ranks.size() > size) {
      bt::framework::get<bt::test_suite>(tu.p_parent_id).remove(tu.p_id);
      return;
    }

    // Install the fixture. Disabled because installing the fixture on just a
    // subset of ranks causes a restriction to be made from a subset of ranks
    // which means the application will hang.
    // tu.p_fixtures.value.push_back(
    //   bt::test_unit_fixture_ptr(
    //     new bt::class_based_fixture<MPICommRestrictFixture, std::vector<int>>(_ranks)));

  }

  virtual bt::decorator::base_ptr clone() const
  {
    return bt::decorator::base_ptr(new OnRanks(_ranks));
  }

  std::vector<int> _ranks;
  
};

/// Boost.Test decorator that makes the test run only on the master aka rank 0
class OnMaster : public OnRanks
{
public:
  explicit OnMaster() :
    OnRanks({0})
  {}
};

/// Boost.Test decorator that makes the test run only on a specific MPI size
class OnSize : public bt::decorator::base
{
public:
  explicit OnSize(const int size) :
    givenSize(size)
  {}
  
  virtual void apply(bt::test_unit& tu)
  {
    if (givenSize != Par::getCommunicatorSize()) {
      bt::framework::get<bt::test_suite>(tu.p_parent_id).remove(tu.p_id);
      return;
    }
  }

  virtual bt::decorator::base_ptr clone() const
  {
    return bt::decorator::base_ptr(new OnSize(givenSize));
  }

  const int givenSize;
};


/// Boost.Test decorator that deletes the test, unless a minimum number of ranks is available.
/*
 * This does not restrict the communicator, which must be done by installing the MPICommrestrictFixture.
 */
class MinRanks : public bt::decorator::base
{
public:
  explicit MinRanks(const int minimumSize) :
    minSize(minimumSize)
  {}
  
private:

  virtual void apply(bt::test_unit& tu)
  {
    if (minSize > Par::getCommunicatorSize()) {
      bt::framework::get<bt::test_suite>(tu.p_parent_id).remove(tu.p_id);
      return;
    }
  }

  virtual bt::decorator::base_ptr clone() const
  {
    return bt::decorator::base_ptr(new MinRanks(minSize));
  }

  const int minSize;
};


/// equals to be used in tests. Prints both operatorans on failure
template<class DerivedA, class DerivedB>
boost::test_tools::predicate_result equals (const Eigen::MatrixBase<DerivedA>& A,
                                            const Eigen::MatrixBase<DerivedB>& B,
                                            double tolerance = math::NUMERICAL_ZERO_DIFFERENCE)
{
  if (not math::equals(A, B, tolerance)) {
    boost::test_tools::predicate_result res(false);
    Eigen::IOFormat format;
    if (A.cols() == 1) {
      format.rowSeparator = ", ";
      format.rowPrefix = "  ";
    }
    res.message() << "\n" << A.format(format) << " != \n" << B.format(format);
    return res;
  }
  return true;
}

/// equals to be used in tests. Prints both operatorans on failure
template<class Scalar>
typename std::enable_if<std::is_arithmetic<Scalar>::value, boost::test_tools::predicate_result>
::type equals(const Scalar a, const Scalar b, const Scalar tolerance = math::NUMERICAL_ZERO_DIFFERENCE)
{
  if (not math::equals(a, b, tolerance))
  {
    boost::test_tools::predicate_result res(false);
    res.message() << "Not equal: " << a << "!=" << b;
    return res;
  }
  return true;
}

}} // namespace precice, testing