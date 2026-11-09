# PC Keyboard Control

## Purpose
Консольное Python-приложение на ПК для управления роботом по Bluetooth: чтение стрелок в raw-режиме, дискретные инкременты ±10%, heartbeat 200 мс, graceful shutdown и автореконнект при обрыве линка.
## Requirements
### Requirement: Console app reads arrow keys without Enter

The application SHALL run a console (terminal) session in raw mode and SHALL recognize the arrow keys Up, Down, Left, Right, and the exit key `q`, reacting to a key press immediately without requiring the Enter key. Terminal contents and behavior SHALL be restored when the application exits.

#### Scenario: Arrow key press is detected immediately

- **WHEN** the user presses an arrow key while the application runs
- **THEN** the application reacts to that key press without waiting for Enter

#### Scenario: Exit key recognized

- **WHEN** the user presses `q` or sends Ctrl+C
- **THEN** the application shuts down and restores the terminal

### Requirement: Discrete speed increments

Starting from `set_speed(0, 0)`, each arrow key press SHALL change the held command by exactly 10% of the configured maximum: Up increments linear speed, Down decrements linear speed, Left increments angular speed, Right decrements angular speed. Linear and angular components SHALL be maintained independently (a forward-and-turn command keeps both components). Each component SHALL be capped so it never exceeds ±100% of its maximum, and Down beyond 0% SHALL continue into negative linear speed (reverse driving).

#### Scenario: Initial command is zero

- **WHEN** the application starts and connects
- **THEN** it sends `set_speed 0.0 0.0` as the initial command

#### Scenario: Linear increment

- **WHEN** the user presses Up once
- **THEN** the linear component becomes +10% of maximum and is sent as the new command

#### Scenario: Linear decrement into reverse

- **WHEN** the user presses Down while linear speed is at +10%
- **THEN** the linear component returns to 0%, and a further Down press drives into negative (reverse) territory

#### Scenario: Angular increment and decrement

- **WHEN** the user presses Left and then Right
- **THEN** the angular component first rises to +10% and then returns to 0% of the angular maximum

#### Scenario: Combined forward and turn

- **WHEN** the user presses Up followed by Left
- **THEN** the command keeps the linear increment and adds the angular increment, resulting in a turn while driving forward

#### Scenario: Cap at maximum

- **WHEN** the user presses Up ten times from zero
- **THEN** the linear component reaches +100% and additional Up presses do not raise it further

### Requirement: Command mapping matches firmware limits

The application SHALL treat +100% linear speed as `V_MAX_MM_S = 300` mm/s and +100% angular speed as `ω_max = 2·300/WHEELBASE_MM ≈ 3.24` rad/s, matching the constants defined in the firmware, and SHALL send numeric values to one decimal place for mm/s and two decimal places for rad/s.

#### Scenario: Ten percent step maps to firmware units

- **WHEN** the user presses Up once
- **THEN** the application sends `set_speed 30.0 0.0` (10% of 300 mm/s)

#### Scenario: Angular step maps to firmware units

- **WHEN** the user presses Left once
- **THEN** the application sends an angular value of approximately 0.32 rad/s (10% of ω_max)

### Requirement: Command repetition as heartbeat

While connected and running, the application SHALL resend the current command at a fixed interval of 200 ms, so the firmware watchdog (which stops the robot after a timeout) is reset, and so a host link drop is automatically detected on the firmware side.

#### Scenario: Steady heartbeat while idle

- **WHEN** the command is unchanged and the application is running
- **THEN** the current command is resent every 200 ms

#### Scenario: Heartbeat tone on each change

- **WHEN** the user changes the command with an arrow key
- **THEN** the new command is sent immediately and continues to be repeated every 200 ms

### Requirement: Graceful shutdown stops the robot

When the application exits, whether via `q`, Ctrl+C, or the reconnection attempts after a link loss being exhausted, it SHALL send a final `set_speed 0.0 0.0` before closing the connection so the robot does not continue moving with the last non-zero command.

#### Scenario: Quit via key sends stop

- **WHEN** the user presses `q`
- **THEN** the application sends `set_speed 0.0 0.0` and then closes the serial port

#### Scenario: Exit via Ctrl+C sends stop

- **WHEN** the user interrupts the application with Ctrl+C
- **THEN** the application sends `set_speed 0.0 0.0` and restores the terminal

#### Scenario: Exit after failed reconnect sends stop

- **WHEN** a link loss cannot be repaired within the reconnection attempts
- **THEN** the application sends `set_speed 0.0 0.0` and exits with an error message

### Requirement: Auto-reconnect on lost link

While running, if the connection to the robot is lost (for example a Bluetooth RFCOMM link that drops), the application SHALL automatically reconnect to the same port and resume sending the current command, keeping the current linear and angular components. It SHALL repeat the reconnection at most 5 times with a 0.5 s pause between attempts; if all attempts fail, the application SHALL exit per the graceful-shutdown requirement. A link loss SHALL not reset the user's held command.

#### Scenario: Link drop while driving resumes

- **WHEN** the connection drops while a non-zero command is held
- **THEN** the application reconnects and continues sending the same command without the user having to rebuild it from zero

#### Scenario: Unrecoverable link loss exits cleanly

- **WHEN** reconnection attempts all fail
- **THEN** the application prints a readable error message and exits with a final stop command

### Requirement: Incoming telemetry is drained while running

While connected, the application SHALL continuously read and discard incoming bytes from the serial port (the bot's keepalive acknowledgements). This keeps the terminal input buffer from filling up, which would otherwise stall the RFCOMM flow control and break the channel over time. Incoming acknowledgement lines SHALL additionally be parsed and rendered live in the telemetry dashboard (separate requirement below), so draining and displaying the data happen together.

#### Scenario: Acknowledgements are drained

- **WHEN** the bot sends `ack ...` lines every ~200 ms while the application is connected
- **THEN** the application reads them promptly so the input buffer does not grow without bound

#### Scenario: Acknowledgements are parsed for display

- **WHEN** a complete `ack` line with `mL/mR/oL/oR/iL/iR` fields arrives
- **THEN** the field values are parsed and pushed into the live telemetry dashboard, and the dashboard is redrawn

#### Scenario: Link closure mid-run is detected via read errors

- **WHEN** the channel closes while driving and a read or write on the port raises an I/O error (for example `EIO` on a dropped RFCOMM link)
- **THEN** the application treats it as a link loss and enters the auto-reconnect flow; an empty read on an idle tty (a no-op return with no data, which is normal on a `VMIN=0` port) is ignored and does not count as a closure

### Requirement: Status line shows absolute command values

The application SHALL display the current command with both the percentage of the maximum and the absolute value: linear speed in mm/s (one decimal place) and angular speed in rad/s (two decimal places), matching the format of the outgoing `set_speed` command.

#### Scenario: Linear value shown

- **WHEN** the linear component is +30%
- **THEN** the status line shows the percentage (`+30%`) together with the absolute value (`90.0 мм/с`)

#### Scenario: Angular value shown

- **WHEN** the angular component is -10%
- **THEN** the status line shows the percentage (`-10%`) together with the absolute value (`-0.32 рад/с`)

### Requirement: Live telemetry dashboard

While connected, the application SHALL run in a full-screen mode using the terminal alternate screen buffer and SHALL continuously render a telemetry dashboard: a status line on top, below it a table of the **50 most recent** acknowledgement lines (newest row on top), and a footer with control hints. The live dashboard SHALL be updated on each new acknowledgement (approximately every 200 ms). The most recent row SHALL be visually highlighted. All internal messages (connection status, link-loss notices, hints) SHALL be rendered inside the dashboard, not written over it. When the application exits, it SHALL leave the alternate screen buffer and restore the normal terminal screen.

Directly above the column header the dashboard SHALL render a two-line legend explaining what every table column means, separated from the column header by one blank line. The legend SHALL state the unit of measure for each column and SHALL distinguish the commanded speed columns from the measured ones. The legend SHALL be static text, SHALL NOT be a table row and SHALL NOT consume table space: the table SHALL still show 50 acknowledgement rows, so the whole panel occupies 58 lines. The column identifiers in the header SHALL remain the same short names as the `ack` protocol fields.

#### Scenario: Dashboard shows last 50 acknowledgements with newest on top

- **WHEN** more than 50 acknowledgements have arrived
- **THEN** the table shows exactly the 50 most recent ones, with the newest row at the top of the table

#### Scenario: Column legend is rendered above the header

- **WHEN** the dashboard is drawn
- **THEN** exactly two legend lines appear immediately above the column header line and below the status line, with one blank line between the legend and the column header, and neither of the legend lines is treated as a table row

#### Scenario: Legend tells commanded apart from measured

- **WHEN** the operator reads the legend
- **THEN** the legend names the unit of measure for each column and marks which columns hold the commanded speed and which hold the measured speed

#### Scenario: Legend does not reduce the number of table rows

- **WHEN** the panel is drawn with the legend present
- **THEN** the table area still occupies exactly 50 rows, padded with empty rows when fewer than 50 acknowledgements have arrived, and the panel is 58 lines tall

#### Scenario: Latest row is highlighted

- **WHEN** the dashboard is drawn
- **THEN** the newest acknowledgement row is visually distinguished (inverted colors)

#### Scenario: Messages render inside the dashboard

- **WHEN** the application needs to notify the user (for example "link lost, reconnecting")
- **THEN** the message is displayed in a slot of the dashboard and the panel is re-rendered cleanly, without mixing with the live table

#### Scenario: Dashboard clears after reconnect

- **WHEN** the connection is re-established after a link loss
- **THEN** the telemetry table is cleared so stale values from the dead link are not shown alongside fresh ones

#### Scenario: Screen restored on exit

- **WHEN** the application exits via `q`, Ctrl+C, or exhausted reconnection attempts
- **THEN** the alternate screen buffer is left, the cursor visibility is restored, and the terminal shows the normal previous content

### Requirement: Serial port is supplied as a launch argument

The application SHALL accept the serial port device as its single command-line argument (for example `/dev/rfcomm0`) and SHALL connect at 115200 baud. If the port is missing or cannot be opened, the application SHALL print a clear error and exit.

#### Scenario: Port provided and opened

- **WHEN** the application is launched with a valid device path
- **THEN** it connects to the device at 115200 baud and starts sending commands

#### Scenario: Port missing

- **WHEN** the application is launched with no argument
- **THEN** it prints usage information and exits without an error trace

#### Scenario: Port cannot be opened

- **WHEN** the device path is invalid or the device is in use
- **THEN** the application prints a readable error message and exits

### Requirement: Space включает паузу

Приложение SHALL распознавать клавишу Space как переключатель паузы/продолжения во время работы. На первое нажатие Space приложение SHALL немедленно перейти в состояние паузы и начать плавный спуск отправляемой уставки к нулю шагами не более 10% от максимума за heartbeat, сохраняя хранимую команду неизменной. На второе нажатие Space оно SHALL выйти из паузы и начать плавный подъём отправляемой уставки от текущего значения к хранимой команде теми же шагами. Первый шаг рампы SHALL отправляться немедленно, последующие — только на очередном heartbeat 200 мс. Нажатие Space без смены состояния паузы — холостой безвредный тумблер, в том числе когда хранимая команда уже нулевая.

#### Scenario: Первое нажатие начинает плавную остановку

- **WHEN** пользователь нажимает Space, пока удерживается ненулевая команда
- **THEN** приложение немедленно отправляет первую ступень уставки в сторону нуля (не более 10% от максимума), переходит в состояние паузы и сохраняет хранимую команду для будущего resume

#### Scenario: Второе нажатие начинает плавный разгон

- **WHEN** пользователь повторно нажимает Space во время паузы
- **THEN** приложение немедленно отправляет первую ступень от текущей уставки к хранимой команде, выходит из паузы и продолжает шаги по heartbeat

#### Scenario: Быстрое двойное нажатие разворачивает рампу

- **WHEN** пользователь нажимает Space второй раз, пока спуск к нулю ещё не завершён
- **THEN** приложение разворачивает уставку от текущего значения обратно к хранимой команде без полной остановки и без ступеньки

#### Scenario: Space при уже нулевой скорости безвреден

- **WHEN** пользователь нажимает Space, пока хранимая команда равна `0.0 0.0`
- **THEN** состояние паузы переключается без движения, и ошибок не возникает

### Requirement: Робот на паузе удерживается на месте

Во время паузы приложение SHALL продолжать heartbeat раз в 200 мс. Пока плавный спуск к нулю не завершён, heartbeat SHALL отправлять очередную ступень уставки (не более 10% от максимума за шаг); после достижения нуля heartbeat SHALL отправлять `set_speed 0.0 0.0`. Прошивка SHALL удерживать нулевую уставку через PID в состоянии `Run`; повторные нулевые команды SHALL NOT переводить привод в `power_stop()` и SHALL NOT сбрасывать интегратор после первого перехода в ноль. Команда `stop` для паузы SHALL NOT использоваться.

#### Scenario: Heartbeat продолжается во время паузы

- **WHEN** приложение на паузе и прошло 200 мс
- **THEN** оно отправляет следующую ступень спуска, а после достижения нуля — `set_speed 0.0 0.0`

#### Scenario: Пауза плавно приводит робота к нулю

- **WHEN** пользователь ставит паузу, пока робот движется
- **THEN** отправляемая уставка уменьшается шагами не более 10% за heartbeat, прошивка остаётся в состоянии `Run` и регулирует обе скорости к нулю, а не выполняет `power_stop()`

#### Scenario: Ноль удерживается после спуска

- **WHEN** спуск к нулю завершён
- **THEN** последующие heartbeat отправляют `set_speed 0.0 0.0`, и прошивка удерживает ноль через PID

### Requirement: Панель замораживается во время паузы

Во время паузы приложение SHALL NOT перерисовывать панель в ответ на входящие подтверждения или клавиши-стрелки: статусная строка и таблица из 50 строк телеметрии SHALL оставаться ровно такими, какими были в момент паузы. Входящие строки подтверждения SHALL по-прежнему читаться, парситься и помещаться в кольцо телеметрии, чтобы таблица сразу после resume показывала свежее состояние.

#### Scenario: Приход ack на паузе не перерисовывает панель

- **WHEN** во время паузы приходит строка `ack`
- **THEN** телеметрия парсится и сохраняется, но панель не перерисовывается

#### Scenario: Стрелки на паузе не перерисовывают панель

- **WHEN** пользователь нажимает стрелку во время паузы
- **THEN** команда не меняется, ничего не отправляется, и панель не перерисовывается

#### Scenario: Панель показывает свежую телеметрию на resume

- **WHEN** пользователь возобновляет движение, а в кольцо телеметрии за время паузы накопились подтверждения
- **THEN** панель перерисовывается на resume и показывает текущие строки подтверждений

### Requirement: Resume продолжает с прежней скорости

При выходе из состояния паузы хранимые линейная и угловая составляющие SHALL остаться без изменений с момента до паузы. Приложение SHALL плавно вести отправляемую уставку от текущего значения к хранимой команде шагами не более 10% от максимума за heartbeat и SHALL NOT менять частоту heartbeat. По достижении хранимой команды обычный heartbeat SHALL продолжаться без изменений.

#### Scenario: Хранимая скорость сохраняется через паузу

- **WHEN** пользователь ставит паузу на линейной +30% и угловой −10%, а затем возобновляет движение
- **THEN** приложение плавно ведёт уставку к `set_speed 90.0 -0.32`, а хранимые компоненты остаются неизменными

#### Scenario: Рампа достигает допаузного значения

- **WHEN** очередная ступень не превышает оставшуюся разницу
- **THEN** отправляемая уставка становится равной хранимой команде и дальше повторяется обычным heartbeat

#### Scenario: Частота отправки не растёт

- **WHEN** идёт плавный выход из паузы
- **THEN** приложение отправляет не более одной команды на heartbeat 200 мс, не считая немедленного первого шага

### Requirement: Обрыв линка во время паузы оставляет робота на паузе

Если связь теряется во время паузы, приложение SHALL пройти существующий поток автореконнекта; при успешном переподключении робот SHALL оставаться на паузе и SHALL NOT возобновлять движение, пока пользователь не нажмёт Space. Хранимая команда SHALL пережить реконнект без изменений.

#### Scenario: Реконнект сохраняет состояние паузы

- **WHEN** линк обрывается во время паузы и успешно переподключается
- **THEN** робот остаётся остановленным, и движение возобновляется только по нажатию Space

#### Scenario: Хранимая команда переживает реконнект

- **WHEN** во время паузы произошли обрыв линка и реконнект
- **THEN** хранимая команда не изменилась и отправляется по Space, возобновляющему движение

### Requirement: Пауза не меняет поведение выхода

Во время паузы `q` и Ctrl+C SHALL по-прежнему корректно завершать приложение, и существующий финальный `set_speed 0.0 0.0` при graceful shutdown SHALL по-прежнему отправляться.

#### Scenario: Выход во время паузы останавливает робота

- **WHEN** пользователь нажимает `q` или Ctrl+C во время паузы
- **THEN** приложение отправляет финальную команду остановки и восстанавливает терминал

### Requirement: Плавная рампа уставки

Приложение SHALL ограничивать изменение отправляемой уставки между последовательными heartbeat. За один шаг модуль каждой компоненты SHALL изменяться не более чем на 10% от её максимума; при одновременном изменении линейной и угловой компонент SHALL применяться общий коэффициент, масштабирующий обе разности одинаково, чтобы шаг не поворачивал вектор изменения уставки. Рампа SHALL считаться в процентах от максимума так, чтобы 1% линейной и 1% угловой скорости давали одинаковый прирост скорости колеса. Первый шаг рампы SHALL отправляться немедленно, последующие — на очередном heartbeat; отдельных таймеров и более частых отправок SHALL NOT быть.

#### Scenario: Шаг ограничен

- **WHEN** цель отличается от текущей уставки больше чем на 10%
- **THEN** каждый heartbeat уменьшает разницу не более чем на 10% от максимума

#### Scenario: Направление шага сохраняется

- **WHEN** во время рампы меняются и линейная, и угловая составляющие
- **THEN** обе разности масштабируются одним коэффициентом, и шаг не поворачивает вектор изменения уставки

#### Scenario: Кривизна при выходе из нуля сохраняется

- **WHEN** рампа идёт от нулевой уставки к цели с обеими ненулевыми компонентами
- **THEN** каждая ступень пропорциональна цели, поэтому отношение `v/ω` остаётся неизменным

#### Scenario: Цель достигается без перерегулирования

- **WHEN** оставшаяся разница не превышает шаг
- **THEN** отправляемая уставка точно равна цели

#### Scenario: Стрелка во время рампы обновляет цель

- **WHEN** пользователь нажимает стрелку, пока идёт рампа
- **THEN** приложение обновляет только цель, не отправляет команду вне очереди, а следующий heartbeat ведёт уставку к новому значению

#### Scenario: Стрелка вне рампы отправляется сразу

- **WHEN** пользователь нажимает стрелку, когда отправляемая уставка уже равна цели
- **THEN** новая команда отправляется немедленно, как и раньше

#### Scenario: Реконнект перезапускает рампу

- **WHEN** связь теряется во время рампы и восстанавливается
- **THEN** приложение сбрасывает текущую уставку в ноль и перезапускает рампу к сохранённой цели, не меняя саму сохранённую команду

#### Scenario: Нулевая цель не создаёт рампу

- **WHEN** хранимая команда равна `0.0 0.0`
- **THEN** отправляемая уставка равна `0.0 0.0`, и промежуточных шагов нет

