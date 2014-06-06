print('AutoExec script')
print(#playlist, ' items in the playlist')
for i = 1, #playlist do
local it = playlist[i]
print(i, it.title, it.file)
end
