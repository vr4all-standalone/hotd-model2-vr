-- Bounded research: only original Test/Service cabinet fields are written.
local root=assert(os.getenv('HOTD_MODEL2_RESEARCH_DIR'))
local m=manager.machine;local cpu=m.devices[':maincpu'];local mem=cpu.spaces.program
local out=assert(io.open(root..'/events.txt','w'));local last=-1
local function set(tag,mask,value)
 for _,field in pairs(assert(m.ioport.ports[tag]).fields)do if field.mask==mask then field:set_value(value);return end end
 error(tag)
end
hotd_service_probe=emu.add_machine_frame_notifier(function()
 local t=m.time:as_double();local sec=math.floor(t)
 if sec~=last then last=sec;out:write(string.format('TIME %.6f IP %08x\n',t,cpu.state['ip'].value));out:flush()
  if sec>=14 then
   local f=assert(io.open(root..string.format('/ram-%03d.bin',sec),'wb'));f:write(mem:read_range(0x500000,0x5fffff,8));f:close()
   for _,screen in pairs(m.screens)do screen:snapshot(root..string.format('/frame-%03d.png',sec));break end
  end
 end
 set(':IN0',4,((t>=16 and t<16.3)or(t>=28 and t<28.3)or(t>=32 and t<32.3)or(t>=40 and t<40.3)or(t>=44 and t<44.3))and 1 or 0)
 local step=t>=18 and t<26.3 and (t-math.floor(t))<.3
 set(':IN0',8,(step or(t>=30 and t<30.3)or(t>=42 and t<42.3))and 1 or 0)
 set(':P1_X',0x3ff,384);set(':P1_Y',0x3ff,233)
 set(':P2_X',0x3ff,379);set(':P2_Y',0x3ff,233)
end)
