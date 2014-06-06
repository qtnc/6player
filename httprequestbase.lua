local rp, rq, buf
local oldprint = print
local newprint
request, response  = {}, {}
debug.getregistry().httprequestbase = function (f, rq1, rp1, buf1)
print('Response building')
rp=rp1; rq=rq1; buf=buf1;
--print = newprint
f()
--print = oldprint
print('End response')
end

function request:__index (name)
return rq[name]
end

function response:__index (name)
return rp[name]
end

function response:__newindex (name, value)
rp[name]=value
end

function newprint (...)
for _, str in ipairs{...} do
buf:append(str)
end end

setmetatable(response,response)
setmetatable(request,request)

print('OK')