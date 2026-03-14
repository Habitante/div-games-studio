PROGRAM test_smoke;
// DIV Games Studio - Smoke test companion program
// Compiled and run by the IDE smoke test (--smoke flag).
// Exercises basic runtime functionality to prove the full
// compile -> run -> verify pipeline works from within the IDE context.
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    result;
BEGIN
    // Basic math (proves the VM runs)
    IF (1 + 1 == 2) passed++; ELSE failed++; END
    IF (10 * 10 == 100) passed++; ELSE failed++; END
    IF (abs(-42) == 42) passed++; ELSE failed++; END

    // String operations (proves the runtime string subsystem works)
    IF (strlen("hello") == 5) passed++; ELSE failed++; END
    IF (strcmp("abc", "abc") == 0) passed++; ELSE failed++; END

    // Memory/variable access (proves global data segment works)
    result = pow(2, 8);
    IF (result == 256) passed++; ELSE failed++; END

    // File I/O (proves the runtime file subsystem works)
    handle = fopen("tests/test_smoke_io.tmp", "w");
    IF (handle >= 0)
        fwrite(OFFSET passed, 1, handle);
        fclose(handle);
        // Read it back
        handle = fopen("tests/test_smoke_io.tmp", "r");
        IF (handle >= 0)
            result = 0;
            fread(OFFSET result, 1, handle);
            fclose(handle);
            IF (result == passed) passed++; ELSE failed++; END
        ELSE
            failed++;
        END
    ELSE
        failed++;
    END

    // Write binary results (2 ints: passed, failed)
    handle = fopen("tests/test_smoke.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END
