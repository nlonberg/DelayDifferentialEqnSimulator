include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(csv_gen csv_gen.cpp)
add_executable(test_simulation test_simulation.cpp simulation.cpp ${PROJECT_BINARY_DIR}/reaction.cpp cell_param.cpp baby_cl.cpp param_set.cpp arg_parse.cpp model.cpp color.cpp observable.cpp csv_reader.cpp analysis.cpp)
add_executable(test_simulation_set test_simulation_set.cpp simulation.cpp ${PROJECT_BINARY_DIR}/reaction.cpp cell_param.cpp baby_cl.cpp param_set.cpp arg_parse.cpp model.cpp color.cpp observable.cpp csv_reader.cpp)
add_executable(test_main test_main.cpp simulation.cpp ${PROJECT_BINARY_DIR}/reaction.cpp cell_param.cpp baby_cl.cpp param_set.cpp arg_parse.cpp model.cpp color.cpp observable.cpp csv_reader.cpp)

add_test(ctest_simulation test_simulation -p ../param_list.csv)
add_test(ctest_simulation_set test_simulation_set -p ../param_list.csv)
add_test(ctest_main test_main -p ../param_list.csv -c 200 -w 50 -s 0.01 -a 10 -t 60 -d ../data_out.csv)



# Forces csv_gen to run before all the test_'s are built.
# This occurs when 'make' is run.
add_custom_target(csv_gen_run ${PROJECT_BINARY_DIR}/source/csv_gen ../)

add_dependencies(test_simulation csv_gen_run)
add_dependencies(test_simulation_set csv_gen_run)
add_dependencies(test_main csv_gen_run)



# Warns user about lack of param_list.csv
if(NOT EXISTS "param_list.csv")
    message(WARNING "Upon running 'make', if it doesn't already exist, 'param_list_template.csv' will be generated in the model directory. Please fill out this template then rename it to 'param_list.csv' before attempting to run any compiled tests or executables.")
endif()