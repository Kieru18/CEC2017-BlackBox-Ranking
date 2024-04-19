# Utwórz cel docelowy dla testów
add_executable(tests testFiles.cc)

# Podłącz do testów bibliotekę Boost
target_link_libraries(tests Boost::unit_test_framework)

# Dodaj testy jako cel testowy w CTest
enable_testing()
add_test(NAME MyTest COMMAND tests)