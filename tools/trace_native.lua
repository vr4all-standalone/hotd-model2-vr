-- Research only: read whole mapped program/RAM, write only real cabinet fields.
local root=assert(os.getenv('HOTD_MODEL2_RESEARCH_DIR'))
local m=manager.machine;local cpu=m.devices[':maincpu'];local mem=cpu.spaces.program
local out=assert(io.open(root..'/events.txt','w'));local last=-1;local sample=-1
local taps={}
for _,range in ipairs({{0x51ec00,0x51ed97},{0x51ee14,0x51ee17}})do
 taps[#taps+1]=mem:install_write_tap(range[1],range[2],'hotd-native-observe',function(a,d,mask)
  if m.time:as_double()>14 then out:write(string.format('WRITE %.6f IP %08x ADDRESS %08x DATA %08x MASK %08x\n',m.time:as_double(),cpu.state['ip'].value,a,d,mask))end
 end)
end
hotd_native_taps=taps
local function press(tag,mask,value)
 for _,field in pairs(assert(m.ioport.ports[tag]).fields)do
  if field.mask==mask then field:set_value(value);return end
 end
 error('Missing native cabinet field '..tag)
end
hotd_observer=emu.add_machine_frame_notifier(function()
 local t=m.time:as_double();local s=math.floor(t);local tick=math.floor(t*10)
 if tick~=sample and t>=14 then sample=tick;local f=assert(io.open(root..string.format('/state-%04d.bin',tick),'wb'));f:write(mem:read_range(0x51ec00,0x51f3ff,8));f:close()end
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
 press(':IN0',1,((t>=16 and t<16.2) or (t>=17 and t<17.2)) and 1 or 0)
 press(':IN0',0x10,(t>=18.5 and t<18.8) and 1 or 0)
 press(':IN0',2,((t>=34 and t<34.2) or (t>=35 and t<35.2)) and 1 or 0)
 press(':IN0',0x20,(t>=36.5 and t<36.8) and 1 or 0)
 press(':IN0',4,(t>=60 and t<60.3) and 1 or 0)
 press(':P1_X',0x3ff,(t>=48 and t<49) and 0 or 384);press(':P1_Y',0x3ff,233)
 press(':P2_X',0x3ff,379);press(':P2_Y',0x3ff,233)
 press(':IN1',1,(t>=32 and t<58 and math.floor(t*2)%2==0) and 1 or 0)
 press(':IN1',2,(t>=40 and t<58 and math.floor(t*2)%2==0) and 1 or 0)
end)
