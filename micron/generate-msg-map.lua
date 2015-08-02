-- generate message map entries:
-- #define ON_WM_SIZE_(fn)		.add_<WM_SIZE, ThisClass>(fn)
-- #define ON_WM_SIZE()			ON_WM_SIZE_(&ThisClass::OnSize)

f = arg[1]

function trim(s)
	return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end

len = 40
tab = 4

print("// Message map macros.\n//\n// Do not edit: generated from " .. arg[1] .. "\n\n")

for name in io.lines(f) do
	name = trim(name)
	if name:len() > 0 then
		m = string.upper(name)
		s = "#define ON_WM_" .. m .. "_(fn)"
		n = math.max(1, math.floor((len - s:len() - 1) / tab))
		print (s .. string.rep("\t", n) .. ".add_<WM_" .. m .. ", ThisClass>((fn))")

		s = "#define ON_WM_" .. m .. "()"
		n = math.max(1, math.floor((len - s:len() - 1) / tab))
		print (s .. string.rep("\t", n) .. "ON_WM_" .. m .. "_(&ThisClass::On" .. name .. ")\n")
	end
end
