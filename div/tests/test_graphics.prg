PROGRAM test_graphics;
GLOBAL
    passed = 0;
    failed = 0;
    handle;
    fpg_id;
    map_id;
    new_id;
    w;
    h;
BEGIN
    // --- load_fpg (first FPG gets file code 0, so we verify via graphic_info) ---
    fpg_id = load_fpg("help/help.fpg");

    // --- graphic_info on FPG graphic ---
    // Graphic 100 is a ball used in help examples
    w = graphic_info(fpg_id, 100, g_wide);
    h = graphic_info(fpg_id, 100, g_height);
    IF (w > 0) passed++; ELSE failed++; END
    IF (h > 0) passed++; ELSE failed++; END

    // --- new_map (create graphic with known dimensions) ---
    new_id = new_map(32, 64, 16, 32, 15);
    IF (new_id >= 1000) passed++; ELSE failed++; END

    // Verify exact dimensions via graphic_info (file 0 for standalone maps)
    IF (graphic_info(0, new_id, g_wide) == 32) passed++; ELSE failed++; END
    IF (graphic_info(0, new_id, g_height) == 64) passed++; ELSE failed++; END
    IF (graphic_info(0, new_id, g_x_center) == 16) passed++; ELSE failed++; END
    IF (graphic_info(0, new_id, g_y_center) == 32) passed++; ELSE failed++; END

    // --- load_map ---
    map_id = load_map("help/help.map");
    IF (map_id >= 1000) passed++; ELSE failed++; END

    w = graphic_info(0, map_id, g_wide);
    h = graphic_info(0, map_id, g_height);
    IF (w > 0) passed++; ELSE failed++; END
    IF (h > 0) passed++; ELSE failed++; END

    // --- unload_map (should not crash) ---
    unload_map(new_id);
    passed++;
    unload_map(map_id);
    passed++;

    // --- unload_fpg (should not crash) ---
    unload_fpg(fpg_id);
    passed++;

    // Write results
    handle = fopen("tests/test_graphics.result", "w");
    fwrite(OFFSET passed, 1, handle);
    fwrite(OFFSET failed, 1, handle);
    fclose(handle);

    exit("", 0);
END
