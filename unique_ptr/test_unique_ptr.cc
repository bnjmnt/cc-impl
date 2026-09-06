#include <gtest/gtest.h>

#include <type_traits>

#include "unique_ptr.h"

namespace {

// Helper to observe destruction.
struct DtorTracker {
  explicit DtorTracker(bool* flag) : destroyed(flag) {}
  ~DtorTracker() { *destroyed = true; }
  bool* destroyed;
};

}  // namespace

// --- Constructor / destructor ---

TEST(UniquePtrTest, DefaultConstructedIsNull) {
  ben::unique_ptr<int> p;
  EXPECT_EQ(p.get(), nullptr);
}

TEST(UniquePtrTest, ConstructedFromRawPointerOwnsIt) {
  int* raw = new int(42);
  ben::unique_ptr<int> p(raw);
  EXPECT_EQ(p.get(), raw);
}

TEST(UniquePtrTest, DestructorDeletesOwnedObject) {
  bool destroyed = false;
  {
    ben::unique_ptr<DtorTracker> p(new DtorTracker(&destroyed));
  }
  EXPECT_TRUE(destroyed);
}

// --- Copy is deleted (compile-time checks) ---

TEST(UniquePtrTest, IsNotCopyable) {
  EXPECT_FALSE(std::is_copy_constructible_v<ben::unique_ptr<int>>);
  EXPECT_FALSE(std::is_copy_assignable_v<ben::unique_ptr<int>>);
}

// --- Move constructor ---

TEST(UniquePtrTest, MoveConstructorTransfersOwnership) {
  int* raw = new int(7);
  ben::unique_ptr<int> a(raw);
  ben::unique_ptr<int> b(std::move(a));

  EXPECT_EQ(b.get(), raw);
  EXPECT_EQ(a.get(), nullptr);  // moved-from must be empty
}

TEST(UniquePtrTest, MoveConstructorDoesNotDoubleDelete) {
  bool destroyed = false;
  {
    ben::unique_ptr<DtorTracker> a(new DtorTracker(&destroyed));
    ben::unique_ptr<DtorTracker> b(std::move(a));
    EXPECT_FALSE(destroyed);  // still alive, ownership just transferred
  }
  EXPECT_TRUE(destroyed);  // destroyed exactly once when b goes out of scope
}

// --- Move assignment ---

TEST(UniquePtrTest, MoveAssignmentTransfersOwnership) {
  int* raw = new int(9);
  ben::unique_ptr<int> a(raw);
  ben::unique_ptr<int> b;

  b = std::move(a);

  EXPECT_EQ(b.get(), raw);
  EXPECT_EQ(a.get(), nullptr);
}

TEST(UniquePtrTest, MoveAssignmentReleasesPreviouslyOwnedObject) {
  bool first_destroyed = false;
  bool second_destroyed = false;
  {
    ben::unique_ptr<DtorTracker> a(new DtorTracker(&first_destroyed));
    ben::unique_ptr<DtorTracker> b(new DtorTracker(&second_destroyed));

    b = std::move(a);

    // b's original object should be destroyed once it's overwritten.
    EXPECT_TRUE(second_destroyed);
    EXPECT_FALSE(first_destroyed);
  }
  EXPECT_TRUE(first_destroyed);
}

TEST(UniquePtrTest, SelfMoveAssignmentIsSafe) {
  bool destroyed = false;
  ben::unique_ptr<DtorTracker> a(new DtorTracker(&destroyed));

  a = std::move(a);

  // Behavior here depends on your self-assignment guard; at minimum
  // this should not crash or double-delete under ASan.
  SUCCEED();
}

// --- noexcept guarantees ---

TEST(UniquePtrTest, MoveOperationsAreNoexcept) {
  EXPECT_TRUE(std::is_nothrow_move_constructible_v<ben::unique_ptr<int>>);
  EXPECT_TRUE(std::is_nothrow_move_assignable_v<ben::unique_ptr<int>>);
}

TEST(UniquePtrTest, ReturnedByValueFromFunction) {
  auto make = []() -> ben::unique_ptr<int> {
    return ben::unique_ptr<int>(new int(3));
  };
  ben::unique_ptr<int> p = make();
  ASSERT_NE(p.get(), nullptr);
  EXPECT_EQ(*p.get(), 3);
}

TEST(UniquePtrTest, GetReturnsNullptrForDefaultConstructed) {
  ben::unique_ptr<int> p;
  EXPECT_EQ(p.get(), nullptr);
}

TEST(UniquePtrTest, GetReturnsOwnedPointer) {
  int* raw = new int(5);
  ben::unique_ptr<int> p(raw);
  EXPECT_EQ(p.get(), raw);
}

TEST(UniquePtrTest, GetDoesNotReleaseOwnership) {
  bool destroyed = false;
  ben::unique_ptr<DtorTracker> p(new DtorTracker(&destroyed));

  DtorTracker* raw = p.get();
  static_cast<void>(raw);  // suppress unused-variable warning

  // Calling get() should not transfer or release ownership —
  // the object should still be alive and owned by p.
  EXPECT_FALSE(destroyed);
}

TEST(UniquePtrTest, GetIsCallableOnConstObject) {
  int* raw = new int(8);
  const ben::unique_ptr<int> p(raw);
  EXPECT_EQ(p.get(), raw);
}

TEST(UniquePtrTest, GetReturnsSamePointerAfterMove) {
  int* raw = new int(11);
  ben::unique_ptr<int> a(raw);
  ben::unique_ptr<int> b(std::move(a));

  EXPECT_EQ(b.get(), raw);
  EXPECT_EQ(a.get(), nullptr);
}

TEST(UniquePtrTest, BoolConversionFalseWhenNull) {
  ben::unique_ptr<int> p;
  EXPECT_FALSE(static_cast<bool>(p));
}

TEST(UniquePtrTest, BoolConversionTrueWhenOwning) {
  ben::unique_ptr<int> p(new int(4));
  EXPECT_TRUE(static_cast<bool>(p));
}

TEST(UniquePtrTest, BoolConversionInIfStatement) {
  ben::unique_ptr<int> p(new int(4));
  if (p) {
    SUCCEED();
  } else {
    FAIL() << "Expected p to be truthy when owning an object";
  }
}

TEST(UniquePtrTest, BoolConversionFalseAfterMove) {
  ben::unique_ptr<int> a(new int(4));
  ben::unique_ptr<int> b(std::move(a));

  EXPECT_FALSE(static_cast<bool>(a));
  EXPECT_TRUE(static_cast<bool>(b));
}

TEST(UniquePtrTest, BoolConversionIsExplicit) {
  // This should fail to compile if operator bool() is not explicit:
  // ben::unique_ptr<int> a(new int(1));
  // ben::unique_ptr<int> b(new int(2));
  // bool result = a < b;  // implicit bool->int comparison, should NOT compile

  EXPECT_TRUE((!std::is_convertible_v<ben::unique_ptr<int>, bool>));
}

namespace {

struct Point {
  int x;
  int y;
  int sum() const { return x + y; }
};

}  // namespace

// --- operator* ---

TEST(UniquePtrTest, DereferenceReturnsReferenceToOwnedObject) {
  ben::unique_ptr<int> p(new int(42));
  EXPECT_EQ(*p, 42);
}

TEST(UniquePtrTest, DereferenceAllowsMutation) {
  ben::unique_ptr<int> p(new int(1));
  *p = 99;
  EXPECT_EQ(*p, 99);
}

TEST(UniquePtrTest, DereferenceOnConstObjectReturnsConstReference) {
  const ben::unique_ptr<int> p(new int(7));
  EXPECT_EQ(*p, 7);
  // *p = 8;  // should fail to compile if const-correct — uncomment to verify
  // manually
}

// --- operator-> ---

TEST(UniquePtrTest, ArrowAccessesMemberOfOwnedObject) {
  ben::unique_ptr<Point> p(new Point{3, 4});
  EXPECT_EQ(p->x, 3);
  EXPECT_EQ(p->y, 4);
}

TEST(UniquePtrTest, ArrowAllowsCallingMemberFunction) {
  ben::unique_ptr<Point> p(new Point{3, 4});
  EXPECT_EQ(p->sum(), 7);
}

TEST(UniquePtrTest, ArrowAllowsMutationOfMember) {
  ben::unique_ptr<Point> p(new Point{0, 0});
  p->x = 10;
  EXPECT_EQ(p->x, 10);
}

TEST(UniquePtrTest, ArrowOnConstObjectAccessesMember) {
  const ben::unique_ptr<Point> p(new Point{5, 6});
  EXPECT_EQ(p->x, 5);
  // p->x = 1;  // should fail to compile if const-correct — uncomment to verify
  // manually
}

TEST(UniquePtrTest, ResetWithNoArgsClearsOwnership) {
  ben::unique_ptr<int> p(new int(1));
  p.reset();
  EXPECT_EQ(p.get(), nullptr);
}

TEST(UniquePtrTest, ResetWithNoArgsDeletesOwnedObject) {
  bool destroyed = false;
  ben::unique_ptr<DtorTracker> p(new DtorTracker(&destroyed));
  p.reset();
  EXPECT_TRUE(destroyed);
}

TEST(UniquePtrTest, ResetWithNewPointerTakesOwnership) {
  ben::unique_ptr<int> p(new int(1));
  int* new_raw = new int(2);
  p.reset(new_raw);
  EXPECT_EQ(p.get(), new_raw);
}

TEST(UniquePtrTest, ResetWithNewPointerDeletesOldObject) {
  bool old_destroyed = false;
  bool new_destroyed = false;
  ben::unique_ptr<DtorTracker> p(new DtorTracker(&old_destroyed));

  p.reset(new DtorTracker(&new_destroyed));

  EXPECT_TRUE(old_destroyed);
  EXPECT_FALSE(new_destroyed);  // the new one should still be alive
}

TEST(UniquePtrTest, ResetOnEmptyUniquePtrIsSafe) {
  ben::unique_ptr<int> p;
  p.reset();  // should not crash
  EXPECT_EQ(p.get(), nullptr);
}

TEST(UniquePtrTest, ResetWithSamePointerIsSafe) {
  int* raw = new int(5);
  ben::unique_ptr<int> p(raw);

  // Resetting with the pointer it already owns should not double-delete.
  // This is a known edge case std::unique_ptr does NOT guard against by
  // default in older implementations — worth checking how you handle it.
  p.reset(raw);

  EXPECT_EQ(p.get(), raw);
}