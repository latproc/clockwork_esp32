# Debounced Input
DONINPUT MACHINE I_Input {
  OPTION stable 50;
  on WHEN I_Input IS on && I_Input.TIMER >= stable;
  off DEFAULT;
}

DOFFINPUT MACHINE I_Input {
  OPTION stable 50;
  off WHEN I_Input IS off && I_Input.TIMER >= stable;
  on DEFAULT;
}

DINPUT MACHINE Input {
  OPTION stable 50;

  off WHEN SELF IS on && Input IS off && Input.TIMER >= stable;
  on WHEN SELF IS on || Input IS on && Input.TIMER >= stable;
  off WHEN SELF IS off || Input IS off && Input.TIMER >= stable;
  off INITIAL;
}
