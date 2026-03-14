PROGRAM test_processes;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    pid1;
    pid2;
    pid3;
    found;
    started = 0;
BEGIN
    // --- Process creation ---
    pid1 = dummy();
    pid2 = dummy();
    pid3 = dummy();

    IF (pid1 <> 0) passed++; ELSE failed++; END
    IF (pid2 <> 0) passed++; ELSE failed++; END
    IF (pid3 <> 0) passed++; ELSE failed++; END

    // All three should be different IDs
    IF (pid1 <> pid2 AND pid2 <> pid3 AND pid1 <> pid3)
        passed++;
    ELSE
        failed++;
    END

    // --- Family tree ---
    // son = most recently created child
    IF (son == pid3) passed++; ELSE failed++; END

    // father of each child = main process
    IF (pid1.father == id) passed++; ELSE failed++; END
    IF (pid2.father == id) passed++; ELSE failed++; END

    // sibling chain
    IF (pid1.smallbro == pid2) passed++; ELSE failed++; END
    IF (pid2.smallbro == pid3) passed++; ELSE failed++; END
    IF (pid3.bigbro == pid2) passed++; ELSE failed++; END
    IF (pid2.bigbro == pid1) passed++; ELSE failed++; END

    // --- Execution: processes ran after FRAME ---
    FRAME;
    IF (started == 3) passed++; ELSE failed++; END

    // --- get_id: count processes of TYPE dummy ---
    found = 0;
    WHILE (get_id(TYPE dummy))
        found++;
    END
    IF (found == 3) passed++; ELSE failed++; END

    // --- signal: kill one process ---
    signal(pid2, s_kill);
    FRAME;

    found = 0;
    WHILE (get_id(TYPE dummy))
        found++;
    END
    IF (found == 2) passed++; ELSE failed++; END

    // --- let_me_alone: kill all others ---
    let_me_alone();
    FRAME;

    found = 0;
    WHILE (get_id(TYPE dummy))
        found++;
    END
    IF (found == 0) passed++; ELSE failed++; END

    // Write results
    handle = fopen("tests/test_processes.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END

PROCESS dummy()
BEGIN
    started++;
    LOOP
        FRAME;
    END
END
