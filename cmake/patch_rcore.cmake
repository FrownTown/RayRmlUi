# Idempotently inserts the rayRmlUi module entry into rcore.c's startup log block,
# immediately after the raudio #endif so it always prints unconditionally.
file(READ "${SRC}" content)
string(FIND "${content}" "rayRmlUi" pos)
if(pos EQUAL -1)
    string(REPLACE
        "not loaded (optional)\");\n#endif\n\n    // Initialize window data"
        "not loaded (optional)\");\n#endif\n    TRACELOG(LOG_INFO, \"    > rayRmlUi:.. loaded (optional)\");\n\n    // Initialize window data"
        content "${content}")
    file(WRITE "${SRC}" "${content}")
    message(STATUS "patch_rcore: inserted rayRmlUi entry")
else()
    message(STATUS "patch_rcore: already applied, skipping")
endif()
