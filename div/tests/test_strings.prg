PROGRAM test_strings;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    STRING s1;
    STRING s2;
    STRING s3;
    val;
BEGIN
    // --- strcpy ---
    strcpy(s1, "hello");
    IF (strcmp(s1, "hello") == 0) passed++; ELSE failed++; END
    strcpy(s1, "world");
    IF (strcmp(s1, "world") == 0) passed++; ELSE failed++; END

    // --- strcat ---
    strcpy(s1, "hello ");
    strcat(s1, "world");
    IF (strcmp(s1, "hello world") == 0) passed++; ELSE failed++; END
    strcpy(s1, "AB");
    strcat(s1, "CD");
    IF (strcmp(s1, "ABCD") == 0) passed++; ELSE failed++; END

    // --- strlen ---
    IF (strlen("hello") == 5) passed++; ELSE failed++; END
    IF (strlen("") == 0) passed++; ELSE failed++; END
    IF (strlen("AB CD") == 5) passed++; ELSE failed++; END

    // --- strcmp ---
    IF (strcmp("abc", "abc") == 0) passed++; ELSE failed++; END
    IF (strcmp("abc", "abd") < 0) passed++; ELSE failed++; END
    IF (strcmp("abd", "abc") > 0) passed++; ELSE failed++; END

    // --- strchr (find any char from set) ---
    IF (strchr("FGHIJK", "AEIOU") == 3) passed++; ELSE failed++; END
    IF (strchr("BCD", "AEIOU") == -1) passed++; ELSE failed++; END

    // --- strstr (find substring) ---
    IF (strstr("hello world", "world") == 6) passed++; ELSE failed++; END
    IF (strstr("hello", "xyz") == -1) passed++; ELSE failed++; END
    IF (strstr("ABCABC", "BC") == 1) passed++; ELSE failed++; END

    // --- upper ---
    strcpy(s1, "hello");
    upper(s1);
    IF (strcmp(s1, "HELLO") == 0) passed++; ELSE failed++; END

    // --- lower ---
    strcpy(s1, "HELLO");
    lower(s1);
    IF (strcmp(s1, "hello") == 0) passed++; ELSE failed++; END

    // --- strdel (delete from start, from end) ---
    strcpy(s1, "ABCDEFGH");
    strdel(s1, 2, 3);
    IF (strcmp(s1, "CDE") == 0) passed++; ELSE failed++; END

    // --- itoa ---
    strcpy(s1, itoa(42));
    IF (strcmp(s1, "42") == 0) passed++; ELSE failed++; END
    strcpy(s1, itoa(-7));
    IF (strcmp(s1, "-7") == 0) passed++; ELSE failed++; END
    strcpy(s1, itoa(0));
    IF (strcmp(s1, "0") == 0) passed++; ELSE failed++; END

    // Write results
    handle = fopen("tests/test_strings.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END
