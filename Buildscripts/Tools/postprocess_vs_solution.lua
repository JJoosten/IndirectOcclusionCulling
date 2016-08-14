local filename = arg[1]
local f = assert(io.open(filename))
local outputEnabled = true
for line in f:lines() do 
	if(outputEnabled == false and string.match(line,"EndProject$")) then
		outputEnabled = true;
	elseif(string.match(line, "Project.*\"ALL_BUILD\"")) then
		outputEnabled = false;
	elseif(outputEnabled == true) then
		print(line)
	end
end


f:close()