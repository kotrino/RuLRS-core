--[[
  Изменение параметров RuLRS

  Лицензия https://www.gnu.org/licenses/gpl-3.0.en.html

  Lua скрипт для радио X7, X9, X-lite и Horus с прошивкой openTx 2.2 или выше

  Оригинальный автор: AlessandroAU + Cruwaller
]] --
local commitSha = '??????'
local shaLUT = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'}
local version = 3;
local gotFirstResp = false -- получен первый ответ
local needResp = false -- нужен ответ
local NewReqTime = 0;
local ReqWaitTime = 100;
local UartGoodPkts = 0; -- хорошие пакеты UART
local UartBadPkts = 0;  -- плохие пакеты UART
local StopUpdate = false; -- остановить обновление
local force_use_lua = false; -- принудительное использование lua
local bindmode = false; -- режим привязки
local wifiupdatemode = false; -- режим обновления по WiFi

local SX127x_RATES = {
    list = {'25Hz(-123dbm)', '50Hz(-120dbm)', '100Hz(-117dbm)', '200Hz(-112dbm)'},
    values = {0x06, 0x05, 0x04, 0x02},
    rates = { 25, 50, 100, 200 },
}
local SX128x_RATES = {
    list = {'50Hz(-117dbm)', '150Hz(-112dbm)', '250Hz(-108dbm)', '500Hz(-105dbm)'},
    values = {0x05, 0x03, 0x01, 0x00},
    rates = { 50, 150, 250, 500 },
}
local tx_lua_version = {
    selected = 1,
    list = {'?', '?', 'v0.3', 'v0.4', 'v0.5'},
    values = {0x01, 0x02, 0x03, 0x04, 0x05},
}

local AirRate = {
    index = 1,
    editable = true,
    name = 'Pkt. Rate',
    selected = 99,
    list = SX127x_RATES.list,
    values = SX127x_RATES.values,
    rates = SX127x_RATES.rates,
    max_allowed = #SX127x_RATES.values,
}

local TLMinterval = {
    index = 2,
    editable = true,
    name = 'TLM Ratio',
    selected = 99,
    list = {'Off', '1:128', '1:64', '1:32', '1:16', '1:8', '1:4', '1:2'},
    values = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
    rates = { 1, 128, 64, 32, 16, 8, 4, 2 },
    max_allowed = 8,
}

local MaxPower = {
    index = 3,
    editable = true,
    name = 'Power',
    selected = 99,
    list =  {'10 mW', '25 mW', '50 mW', '100 mW', '250 mW', '500 mW', '1000 mW', '2000 mW'},
    values = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
    max_allowed = 8,
}

local RFfreq = {
    index = 4,
    editable = false,
    name = 'RF Freq',
    selected = 99,
    list = {'915 AU', '915 FCC', '868 EU', '433 AU', '433 EU', '2.4G ISM', '866 IN'},
    values = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
    max_allowed = 7,
}

local function binding(item, event)
    if (bindmode == true) then
        crossfireTelemetryPush(0x2D, {0xEE, 0xEA, 0xFF, 0x00})
    else
        crossfireTelemetryPush(0x2D, {0xEE, 0xEA, 0xFF, 0x01})
    end

    playTone(2000, 50, 0)
    item.exec = false
    return 0
end

-- Кнопка привязки
local Bind = {
    index = 5,
    editable = false,
    name = '[Bind]',
    exec = false,
    func = binding,
    selected = 99,
    list = {},
    values = {},
    max_allowed = 0,
    offsets = {left=5, right=0, top=5, bottom=5},
}

-- Функция запуска веб-сервера
local function web_server_start(item, event)
    crossfireTelemetryPush(0x2D, {0xEE, 0xEA, 0xFE, 0x01})
    playTone(2000, 50, 0)
    item.exec = false
    return 0
end

-- Кнопка обновления по WiFi
local WebServer = {
    index = 5,
    editable = false,
    name = '[Wifi Update]',
    exec = false,
    func = web_server_start,
    selected = 99,
    list = {},
    values = {},
    max_allowed = 0,
    offsets = {left=65, right=0, top=5, bottom=5},
}

-- local exit_script = {
    -- index = 7,
    -- editable = false,
    -- action = 'exit',
    -- name = '[EXIT]',
    -- selected = 99,
    -- list = {},
    -- values = {},
    -- max_allowed = 0,
    -- offsets = {left=5, right=0, top=5, bottom=5},
-- }

local menu = {
    selected = 1,
    modify = false,
    Примечание: индексы списка должны соответствовать обработке параметров в tx_main!
    list = {AirRate, TLMinterval, MaxPower, RFfreq, Bind, WebServer},
    --list = {AirRate, TLMinterval, MaxPower, RFfreq, WebServer, exit_script},
}

-- Включить принудительное использование lua
local function force_use_lua_enable()
    force_use_lua = true
    playTone(2000, 50, 0)
end

-- Возвращает флаги для lcd.drawText для инвертированного и мигающего текста
local function getFlags(element)
    if menu.selected ~= element then return 0 end
    if menu.selected == element and menu.modify == false then
        StopUpdate = false
        return 0 + INVERS
    end
    -- Этот элемент сейчас выбран
    StopUpdate = true
    return 0 + INVERS + BLINK
end

-- ################################################

local supportedRadios =
{
    ["128x64"]  =
    {
        --highRes         = false,
        textSize        = SMLSIZE,
        xOffset         = 60,
        yOffset         = 8,
        yOffset_val     = 3,
        topOffset       = 1,
        leftOffset      = 1,
    },
    ["128x96"]  =
    {
        --highRes         = false,
        textSize        = SMLSIZE,
        xOffset         = 60,
        yOffset         = 8,
        yOffset_val     = 3,
        topOffset       = 1,
        leftOffset      = 1,
    },
    ["212x64"]  =
    {
        --highRes         = false,
        textSize        = SMLSIZE,
        xOffset         = 60,
        yOffset         = 8,
        yOffset_val     = 3,
        topOffset       = 1,
        leftOffset      = 1,
    },
    ["480x272"] =
    {
        --highRes         = true,
        textSize        = 0,
        xOffset         = 100,
        yOffset         = 20,
        yOffset_val     = 5,
        topOffset       = 1,
        leftOffset      = 1,
    },
    ["320x480"] =
    {
        --highRes         = true,
        textSize        = 0,
        xOffset         = 120,
        yOffset         = 25,
        yOffset_val     = 5,
        topOffset       = 5,
        leftOffset      = 5,
    },
}

local radio_resolution = LCD_W.."x"..LCD_H
local radio_data = assert(supportedRadios[radio_resolution], radio_resolution.." not supported")

-- Обновление экрана
local function refreshLCD()

    local yOffset = radio_data.topOffset;
    local lOffset = radio_data.leftOffset;

    lcd.clear()
    if wifiupdatemode == true then --make this less hacky later
        lcd.drawText(lOffset, yOffset, "Goto http://10.0.0.1   ", INVERS)
    -- elseif bindmode == true then
    else
        lcd.drawText(lOffset, yOffset, 'RuLRS ' .. commitSha .. '  ' .. tostring(UartBadPkts) .. ':' .. tostring(UartGoodPkts), INVERS)
    end

    if tx_lua_version.values[tx_lua_version.selected] == version or force_use_lua == true then
        yOffset = radio_data.yOffset_val
        for idx,item in pairs(menu.list) do
            local offsets = {left=0, right=0, top=0, bottom=0}
            if item.offsets ~= nil then
                offsets = item.offsets
            end
            lOffset = offsets.left + radio_data.leftOffset
            local item_y = yOffset + offsets.top + radio_data.yOffset * item.index
            if item.action ~= nil or item.func ~= nil then
                lcd.drawText(lOffset, item_y, item.name, getFlags(idx) + radio_data.textSize)
            else
                local value = '?'
                if 0 < item.selected and item.selected <= #item.list and gotFirstResp then
                --if 0 < item.selected and item.selected <= #item.list and item.selected <= item.max_allowed then
                    value = item.list[item.selected]
                    -- Apply the view function to the value if present
                    if item.view ~= nil then
                        value = item.view(item, value)
                    end
                end
                lcd.drawText(lOffset, item_y, item.name, radio_data.textSize)
                lcd.drawText(radio_data.xOffset, item_y, value, getFlags(idx) + radio_data.textSize)
            end
        end
    elseif gotFirstResp then
        lcd.drawText(lOffset, (radio_data.yOffset*2), "!!! VERSION MISMATCH !!!", INVERS)
        if (tx_lua_version.values[tx_lua_version.selected] > version) then
            lcd.drawText(lOffset, (radio_data.yOffset*3), "Update RULRS.lua", INVERS)
        else
            lcd.drawText(lOffset, (radio_data.yOffset*3), "Update TX module", INVERS)
        end
        lcd.drawText(lOffset, (radio_data.yOffset*4), "LUA v0."..version..", TX "..tx_lua_version.list[tx_lua_version.selected], INVERS)
        lcd.drawText(lOffset, (radio_data.yOffset*5), "[force use]", INVERS + BLINK)
    elseif version == -1 then
        lcd.drawText(lOffset, (radio_data.yOffset*2), "!!! VERSION MISMATCH !!!", INVERS)
        lcd.drawText(lOffset, (radio_data.yOffset*3), "Module is not RULRS v1", INVERS)
        lcd.drawText(lOffset, (radio_data.yOffset*5), "Use the rulrsV2.lua", INVERS)
    else
        lcd.drawText(lOffset, (radio_data.yOffset*5), "Connecting...", INVERS + BLINK)
    end
end

local function increase(_menu)
    local item = _menu
    if item.modify then
        item = item.list[item.selected]
    end

    if item.selected < #item.list and
       (item.max_allowed == nil or item.selected < item.max_allowed) then
        item.selected = item.selected + 1
        --playTone(2000, 50, 0)
    end
end

local function decrease(_menu)
    local item = _menu
    if item.modify then
        item = item.list[item.selected]
    end
    if item.selected > 1 and item.selected <= #item.list then
        item.selected = item.selected - 1
        --playTone(2000, 50, 0)
    end
end

-- ################################################

--[[
Неясно, как работает система push/pop телеметрии. Мы не всегда получаем
ответ на одно push-событие. Могут ли накапливаться несколько ответов? Есть ли у них таймаут?

Если есть несколько ответов, обычно нам нужен самый новый, поэтому этот метод
будет читать до получения nil-ответа, отбрасывая старые данные. Используется максимальное 
количество чтений для защиты от возможности длительного выполнения этой функции.
]]--

function GetIndexOf(t,val)
    for k,v in ipairs(t) do
        if v == val then
            return k
        end
    end
end

-- Функция отображения интервала телеметрии
local function viewTlmInterval(item, value)
    -- Рассчитываем пакетную скорость телеметрии так же, как она определена в rx_main
    local TELEM_MIN_LINK_INTERVAL = 512 -- определено в rx_main, мс на пакет связи
    local hz = AirRate.rates[AirRate.selected]
    local ratiodiv = TLMinterval.rates[TLMinterval.selected]
    local burst = math.floor(math.floor(TELEM_MIN_LINK_INTERVAL * hz / ratiodiv) / 1000)
    -- Резервируем один слот для телеметрии LINK
    burst = (burst > 1) and (burst - 1) or 1
    -- Рассчитываем пропускную способность используя пакеты в секунду и burst
    local telemPPS = hz / ratiodiv
    local bandwidth = math.floor(5 * 8 * telemPPS * burst / (burst + 1) + 0.5)

    if ratiodiv == 1 then
        return value
    else
        return string.format("%s (%dbps)", value, bandwidth)
    end
end

-- Загрузка функций отображения
local function loadViewFunctions()
    TLMinterval.view = viewTlmInterval
end

-- Обработка ELRS версии 2
local function processElrsV2(data)
    UartBadPkts = data[3]
    UartGoodPkts = (data[4]*256) + data[5]
    version = -1 -- Указывает что это система RULRS 2.0, неверный скрипт
end

-- Обработка ответа
local function processResp()
    local command, data = crossfireTelemetryPop()
    if (data == nil) then return end

    if (command == 0x2D) and (data[1] == 0xEA) and (data[2] == 0xEE) then
        -- Тип 0xff - "sendLuaParams"
        if( data[3] == 0xFF) then
            gotFirstResp = true

            if (#data == 12 or force_use_lua == true) then
                bindmode = bit32.btest(0x01, data[4]) -- bind mode active
                wifiupdatemode = bit32.btest(0x02, data[4])
                if StopUpdate == false then
                    TLMinterval.selected = GetIndexOf(TLMinterval.values,data[6])
                    MaxPower.selected = GetIndexOf(MaxPower.values,data[7])
                    tx_lua_version.selected = GetIndexOf(tx_lua_version.values,data[12])
                    if data[8] == 6 then
                        -- ISM 2400 band (SX128x)
                        AirRate.list = SX128x_RATES.list
                        AirRate.rates = SX128x_RATES.rates
                        AirRate.values = SX128x_RATES.values
                        AirRate.max_allowed = #SX128x_RATES.values
                    else
                        -- 433/868/915 (SX127x)
                        AirRate.list = SX127x_RATES.list
                        AirRate.rates = SX127x_RATES.rates
                        AirRate.values = SX127x_RATES.values
                        AirRate.max_allowed = #SX127x_RATES.values
                    end
                    RFfreq.selected = GetIndexOf(RFfreq.values,data[8])
                    AirRate.selected =  GetIndexOf(AirRate.values, data[5])
                end

                UartBadPkts = data[9]
                UartGoodPkts = data[10] * 256 + data[11]
            end -- if correct amount of data for version

        -- Type 0xfe - "luaCommitPacket"
        elseif(data[3] == 0xFE) and #data == 9 then
            commitSha = shaLUT[data[4]+1] .. shaLUT[data[5]+1] .. shaLUT[data[6]+1] .. shaLUT[data[7]+1] .. shaLUT[data[8]+1] .. shaLUT[data[9]+1]
        end

        needResp = false
    elseif (command == 0x2E) and (data[1] == 0xEA) and (data[2] == 0xEE) then
        processElrsV2(data)
    end
end

local function init_func()
    loadViewFunctions()
end

local function bg_func(event)
end

--[[
  Вызывается через (неопределенные) интервалы когда скрипт запущен и экран видим

  Обрабатывает нажатия клавиш и отправляет изменения состояния в tx модуль.

  Базовая стратегия:
    прочитать все ожидающие данные телеметрии
    обработать событие, отправляя telemetryPush при необходимости
    если не было push из-за событий, отправить пустой push чтобы обеспечить отправку текущих значений для следующей итерации
    обновить экран
]]--

local function run_func(event)
    if (gotFirstResp == false or commitSha == '??????') and (getTime() > (NewReqTime + ReqWaitTime)) then
        crossfireTelemetryPush(0x2D, {0xEE, 0xEA, 0x00, 0x00}) -- пингуем пока не получим ответ
        NewReqTime = getTime()
    end

    if needResp == true and (getTime() > (NewReqTime + ReqWaitTime)) then
        crossfireTelemetryPush(0x2D, {0xEE, 0xEA, 0x00, 0x00}) -- пингуем пока не получим ответ
        NewReqTime = getTime()
    end

    processResp() -- проверяем есть ли данные от модуля

    local type = menu.selected
    local item = menu.list[type]

    if item.exec == true and item.func ~= nil then
        local retval = item.func(item, event)
        refreshLCD()
        return retval
    end

    -- обрабатываем нажатия клавиш
    if event == EVT_VIRTUAL_ENTER_LONG or
       event == EVT_ENTER_LONG or
       event == EVT_MENU_LONG then
        -- выход из скрипта
        return 2
    elseif event == EVT_VIRTUAL_PREV or
           event == EVT_VIRTUAL_PREV_REPT or
           event == EVT_ROT_LEFT or
           --event == EVT_MINUS_BREAK or
           event == EVT_SLIDE_LEFT then
        decrease(menu)
    elseif event == EVT_VIRTUAL_NEXT or
           event == EVT_VIRTUAL_NEXT_REPT or
           event == EVT_ROT_RIGHT or
           --event == EVT_PLUS_BREAK or
           event == EVT_SLIDE_RIGHT then
        increase(menu)
    elseif event == EVT_VIRTUAL_ENTER or
           event == EVT_ENTER_BREAK then
        if version ~= tx_lua_version.values[tx_lua_version.selected] and force_use_lua == false then
            force_use_lua_enable()
        elseif menu.modify then
            -- обновить модуль когда редактирование завершено
            local value = 0
            if 0 < item.selected and item.selected <= #item.values then
                value = item.values[item.selected]
            else
                type = 0
            end
            crossfireTelemetryPush(0x2D, {0xEE, 0xEA, type, value})
            NewReqTime = getTime()
            needResp = true
            menu.modify = false
        elseif item.editable and 0 < item.selected and item.selected <= #item.values then
            -- разрешить модификацию только если не readonly и значения получены от модуля
            menu.modify = true
        elseif item.func ~= nil then
            item.exec = true
        elseif item.action == 'exit' then
            -- выход из скрипта
            return 2
        end
    elseif menu.modify and (event == EVT_VIRTUAL_EXIT or
                           event == EVT_EXIT_BREAK or
                           event == EVT_RTN_FIRST) then
        menu.modify = false
        crossfireTelemetryPush(0x2D, {0xEE, 0xEA, 0x00, 0x00}) -- обновить данные
        NewReqTime = getTime()
        needResp = true
    end

    refreshLCD()
    return 0
end

return {run = run_func, init = init_func}

local function parseRuLRSInfoMessage(data)
  if data[2] ~= deviceId then
    fieldData = nil
    fieldChunk = 0
    return
  end
end

local function parseRuLRSV1Message(data)
end
