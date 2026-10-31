-- Research only: read whole mapped program/RAM, write only real cabinet fields.
local root=assert(os.getenv('HOTD_MODEL2_RESEARCH_DIR'))
local m=manager.machine;local cpu=m.devices[':maincpu'];local mem=cpu.spaces.program
local out=assert(io.open(root..'/events.txt','w'));local last=-1
local function press(tag,mask,value)
 for _,field in pairs(assert(m.ioport.ports[tag]).fields)do
  if field.mask==mask then field:set_value(value);return end
 end
 error('Missing native cabinet field '..tag)
end
hotd_observer=emu.add_machine_frame_notifier(function()
 local t=m.time:as_double();local s=math.floor(t)
 if s~=last then
  last=s;out:write(string.format('TIME %.6f IP %08x\n',t,cpu.state['ip'].value));out:flush()
  if s==5 then
   local program=assert(io.open(root..'/program.bin','wb'));program:write(mem:read_range(0,0x1fffff,8));program:close()
  end
  if s%5==0 then
   local ram=assert(io.open(root..string.format('/ram-%03d.bin',s),'wb'));ram:write(mem:read_range(0x500000,0x5fffff,8));ram:close()
   for _,screen in pairs(m.screens)do screen:snapshot(root..string.format('/frame-%03d.png',s));break end
  end
 end
 press(':IN0',1,(t>=16 and t<16.2) and 1 or 0)
 press(':IN0',0x10,(t>=16.5 and t<16.8) and 1 or 0)
 press(':IN0',2,(t>=25 and t<25.2) and 1 or 0)
 press(':IN0',0x20,(t>=25.5 and t<25.8) and 1 or 0)
 press(':P1_X',0x3ff,384);press(':P1_Y',0x3ff,233)
 press(':P2_X',0x3ff,379);press(':P2_Y',0x3ff,233)
 press(':IN1',1,(t>=28 and math.floor(t*2)%2==0) and 1 or 0)
 press(':IN1',2,(t>=30 and math.floor(t*2)%2==0) and 1 or 0)
end)
