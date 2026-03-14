PROGRAM test_files;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    table1[3];
    table2[3];
    val;
    pos;
    len;
BEGIN
    // --- Write and read back ---
    table1[0] = 11;
    table1[1] = 22;
    table1[2] = 33;
    table1[3] = 44;

    handle = fopen("tests/test_files.dat", "w");
    IF (handle <> 0) passed++; ELSE failed++; END
    fwrite(OFFSET table1, 4, handle);
    fclose(handle);

    handle = fopen("tests/test_files.dat", "r");
    IF (handle <> 0) passed++; ELSE failed++; END
    fread(OFFSET table2, 4, handle);
    fclose(handle);

    IF (table2[0] == 11) passed++; ELSE failed++; END
    IF (table2[1] == 22) passed++; ELSE failed++; END
    IF (table2[2] == 33) passed++; ELSE failed++; END
    IF (table2[3] == 44) passed++; ELSE failed++; END

    // --- ftell ---
    handle = fopen("tests/test_files.dat", "r");
    pos = ftell(handle);
    IF (pos == 0) passed++; ELSE failed++; END

    fread(OFFSET val, 1, handle);
    pos = ftell(handle);
    IF (pos == 1) passed++; ELSE failed++; END
    IF (val == 11) passed++; ELSE failed++; END
    fclose(handle);

    // --- fseek (seek_set) ---
    handle = fopen("tests/test_files.dat", "r");
    fseek(handle, 2, seek_set);
    fread(OFFSET val, 1, handle);
    IF (val == 33) passed++; ELSE failed++; END
    fclose(handle);

    // --- fseek (seek_cur) ---
    handle = fopen("tests/test_files.dat", "r");
    fread(OFFSET val, 1, handle);
    fseek(handle, 1, seek_cur);
    fread(OFFSET val, 1, handle);
    IF (val == 33) passed++; ELSE failed++; END
    fclose(handle);

    // --- filelength ---
    handle = fopen("tests/test_files.dat", "r");
    len = filelength(handle);
    IF (len == 4) passed++; ELSE failed++; END
    fclose(handle);

    // Write results
    handle = fopen("tests/test_files.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END
