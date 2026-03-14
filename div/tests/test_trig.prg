PROGRAM test_trig;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    val;
BEGIN
    // --- sin (millidegrees, returns -1000..1000) ---
    IF (sin(0) == 0) passed++; ELSE failed++; END
    IF (sin(90000) == 1000) passed++; ELSE failed++; END
    IF (sin(180000) == 0) passed++; ELSE failed++; END
    IF (sin(270000) == -1000) passed++; ELSE failed++; END

    // --- cos (millidegrees, returns -1000..1000) ---
    IF (cos(0) == 1000) passed++; ELSE failed++; END
    IF (cos(90000) == 0) passed++; ELSE failed++; END
    IF (cos(180000) == -1000) passed++; ELSE failed++; END
    IF (cos(270000) == 0) passed++; ELSE failed++; END

    // --- fget_dist (Euclidean distance between two points) ---
    IF (fget_dist(0, 0, 3, 4) == 5) passed++; ELSE failed++; END
    IF (fget_dist(0, 0, 0, 0) == 0) passed++; ELSE failed++; END
    IF (fget_dist(0, 0, 100, 0) == 100) passed++; ELSE failed++; END
    IF (fget_dist(0, 0, 0, 100) == 100) passed++; ELSE failed++; END

    // --- fget_angle (angle from point A to point B in millidegrees) ---
    // Right = 0, Up = 90000, Left = 180000, Down = -90000
    IF (fget_angle(0, 0, 100, 0) == 0) passed++; ELSE failed++; END
    IF (fget_angle(0, 0, 0, -100) == 90000) passed++; ELSE failed++; END
    IF (fget_angle(0, 0, 0, 100) == -90000) passed++; ELSE failed++; END
    IF (abs(fget_angle(0, 0, -100, 0)) == 180000) passed++; ELSE failed++; END

    // --- get_distx / get_disty (vector decomposition) ---
    // At 0 degrees: full rightward
    IF (get_distx(0, 100) == 100) passed++; ELSE failed++; END
    IF (get_disty(0, 100) == 0) passed++; ELSE failed++; END
    // At 90 degrees: full upward (negative Y on screen)
    IF (get_distx(90000, 100) == 0) passed++; ELSE failed++; END
    IF (get_disty(90000, 100) == -100) passed++; ELSE failed++; END
    // At 180 degrees: full leftward
    IF (get_distx(180000, 100) == -100) passed++; ELSE failed++; END
    IF (get_disty(180000, 100) == 0) passed++; ELSE failed++; END

    // --- near_angle (move angle toward target by increment) ---
    IF (near_angle(0, 90000, 10000) == 10000) passed++; ELSE failed++; END
    IF (near_angle(90000, 90000, 10000) == 90000) passed++; ELSE failed++; END
    IF (near_angle(80000, 90000, 50000) == 90000) passed++; ELSE failed++; END

    // Write results
    handle = fopen("tests/test_trig.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END
