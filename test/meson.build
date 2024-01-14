project('stm32x', 'cpp', default_options: ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

inc = [
  '../include',
  ]

test_src = [
  'test_storage.cc',
  'test_sector_detail.cc',
  'stm32x_test.cc'
  ]

src = [
  '../src/util/util_storage.cc'
  ]

extern_src = [
  ]

gtest_dep = dependency('gtest', main : true, required: true)
fmt_dep = dependency('fmt', required: true)

stm32x_test = executable(
  'stm32x_test',
  cpp_args : [ '-Wno-gnu-zero-variadic-macro-arguments', '-DSTM32X_TESTING' ],
  sources : [ test_src, src, extern_src ],
  include_directories : inc,
  dependencies : [ gtest_dep, fmt_dep ])

test('stm32x_test', stm32x_test)
