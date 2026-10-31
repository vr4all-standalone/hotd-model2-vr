-- Research-only native coin timing. Existing emulator/media stay read-only.
-- Optional HOTD_COIN_DENOMINATOR=2 changes only a derived operator coinage
-- field in this disposable process; it never changes the user's source files.
local root=assert(os.getenv('HOTD_MODEL2_RESEARCH_DIR'))
local m=manager.machine;local mem=m.devices[':maincpu'].spaces.program
local out=assert(io.open(root..'/coins.csv','w'));out:write('time,coin1,coins_received,credits,remainder,fraction,denominator,start,mode,p1\n')
local intervals={{16,.1},{16.2,.1},{20,.15},{20.3,.15},{24,.2},{24.4,.2}}
local configured=false;local last='';local denom=tonumber(os.getenv('HOTD_COIN_DENOMINATOR')or'0')
local function set(mask,value)
 for _,f in pairs(m.ioport.ports[':IN0'].fields)do if f.mask==mask then f:set_value(value);return end end
 error('IN0')
end
hotd_coin_timing=emu.add_machine_frame_notifier(function()
 local t=m.time:as_double()
 if denom>0 and not configured and t>=15 then mem:write_u16(0x51e192,denom);configured=true end
 local coin=false;for _,v in ipairs(intervals)do coin=coin or(t>=v[1]and t<v[1]+v[2])end
 local start=t>=27 and t<27.3
 set(1,coin and 1 or 0);set(0x10,start and 1 or 0)
 local state=string.format('%d,%d,%d,%d,%d,%d,%d,%d,%d',coin and 1 or 0,mem:read_u16(0x1d0000e),mem:read_u32(0x51ee38),mem:read_u32(0x51ee40),mem:read_u16(0x51e1ac),mem:read_u16(0x51e192),start and 1 or 0,mem:read_u16(0x51ee14),mem:read_u32(0x51ec04))
 if state~=last and t>=14 then last=state;out:write(string.format('%.6f,%s\n',t,state));out:flush()end
end)
