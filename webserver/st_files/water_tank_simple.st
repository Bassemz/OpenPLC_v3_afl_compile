PROGRAM water_tank
  VAR
    tank_level AT %QW0 : INT := 500;
    pump_state AT %QW1 : INT := 0;
    alarm_state AT %QW2 : INT := 0;
    setpoint AT %QW3 : INT := 500;
  END_VAR

  IF pump_state = 1 THEN
    tank_level := tank_level + 5;
  END_IF;

  IF tank_level > 0 THEN
    tank_level := tank_level - 2;
  END_IF;

  IF tank_level > 1000 THEN
    tank_level := 1000;
  END_IF;

  IF tank_level < 0 THEN
    tank_level := 0;
  END_IF;

  IF tank_level < 300 THEN
    pump_state := 1;
  END_IF;

  IF tank_level > 700 THEN
    pump_state := 0;
  END_IF;

  IF tank_level > 850 THEN
    alarm_state := 1;
  END_IF;

  IF tank_level < 800 THEN
    alarm_state := 0;
  END_IF;

END_PROGRAM
