#include <iostream>
#include "../src/helpers.h"
#include "catch.hpp"

SCENARIO("L2 distance computation") {
  GIVEN("4 scalar values") {
    double x1, y1, x2, y2;

    WHEN("both 2D points are identical") {
      x1 = 1;
      y1 = 1;
      double d_ = dist2(x1, y1, x1, y1);
      THEN("no error is thrown") { REQUIRE(d_ == 0); }
    }

    WHEN("both 2D points are different") {
      x1 = 1;
      y1 = 1;
      x2 = 2;
      y2 = 2;
      double d_ = dist2(x1, y1, x2, y2);
      THEN("no error is thrown") { REQUIRE(d_ == 2); }
    }
  }
}

SCENARIO("Error computation") {
  GIVEN("6 scalar values") {
    double x1, y1, t1, x2, y2, t2;

    WHEN("both particles are identical") {
      x1 = 1;
      y1 = 1;
      t1 = 1;
      double* e_ = getError(x1, y1, t1, x1, y1, t1);
      THEN("no error is thrown") {
        REQUIRE(e_[0] == 0);
        REQUIRE(e_[1] == 0);
        REQUIRE(e_[2] == 0);
      }
    }

    WHEN("both particles are different") {
      x1 = 1;
      y1 = 1;
      t1 = 2 * M_PI;
      x2 = 2;
      y2 = 2;
      t2 = 0;
      double* e_ = getError(x1, y1, t1, x2, y2, t2);
      THEN("no error is thrown") {
        REQUIRE(e_[0] == 1);
        REQUIRE(e_[1] == 1);
        REQUIRE(e_[2] == 0);
      }
    }
  }
}
