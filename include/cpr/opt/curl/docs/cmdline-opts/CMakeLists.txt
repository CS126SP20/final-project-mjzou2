set(MANPAGE "${CMAKE_BINARY_DIR}/docs/curl.1")

# Load DPAGES and OTHERPAGES from shared file
transform_makefile_inc("Makefile.inc" "${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake")
include("${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake")

add_custom_command(OUTPUT "${MANPAGE}"
  COMMAND "${PERL_EXECUTABLE}" "gen.pl" mainpage "." > "${MANPAGE}"
  DEPENDS ${DPAGES} ${OTHERPAGES}
  VERBATIM
)
add_custom_target(generate-curl.1 DEPENDS "${MANPAGE}")
