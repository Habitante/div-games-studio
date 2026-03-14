PROGRAM test_math;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    result;
    i;
BEGIN
    // --- abs() ---
    IF (abs(5) == 5) passed++; ELSE failed++; END
    IF (abs(-5) == 5) passed++; ELSE failed++; END
    IF (abs(0) == 0) passed++; ELSE failed++; END

    // --- pow() ---
    IF (pow(2, 10) == 1024) passed++; ELSE failed++; END
    IF (pow(3, 3) == 27) passed++; ELSE failed++; END
    IF (pow(5, 0) == 1) passed++; ELSE failed++; END

    // --- sqrt() ---
    IF (sqrt(100) == 10) passed++; ELSE failed++; END
    IF (sqrt(0) == 0) passed++; ELSE failed++; END
    IF (sqrt(1) == 1) passed++; ELSE failed++; END

    // --- rand() range ---
    FROM i = 0 TO 99;
        result = rand(1, 10);
        IF (result < 1 OR result > 10)
            failed++;
        ELSE
            passed++;
        END
    END

    // Write binary results (2 ints: passed, failed)
    handle = fopen("tests/test_math.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END
