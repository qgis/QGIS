IFS=:
cat <<EOF |
access:accessing
adapt:adapting
application:applying
apply:applying
change:changing
choose:choosing
combine:combining
continue:continuing
control:controlling
convert:converting
create:creating
customize:customizing
define:defining
determine:determining
disable:disabling
discard:discarding
display:displaying
drop:dropping
edit and save python file:editing and saving python files
edit:editing
enable:enabling
enter:entering
estimate:estimating
exit:exiting
export:exporting
extract:extracgin
filter:filtering
fix:fixing
first sub-sample:sub-sampling first
fuse:fusing
generate:generating
handle:handling
input:inputting
insert:inserting
interactively manipulate:interactive manipulation
manage:managing
map:mapping
move:moving
ortho-rectify:ortho-rectifying
parametrize:parametrizing
parse:parsing
perform:performing
performs:performing
reduce:reducing
remove:removing
render:rendering
reproject and rasterize:reprojecting and rasterizing
reproject:reprojecting
restrict:restricting
retrieve:retrieving
save:saving
scroll:scrolling
select:selecting
selecting:selecting
set:setting
shorten:shortening
simplify:simplifying
speed-up:speeding up
split:splitting
store:storing
surpress:surpressing
use:using
write:writing
you to show:showing
compute:computing
optimize:optimizing
check:checking
quickly edit:quick editing
EOF
while read v i; do
	echo "$v => $i"
	git grep -l "allows to $v" >files
	[ -s files ] && xargs perl -i.bak -pe "s/allows to $v\b/allows $i/g" <files
done

git grep "allows to" | sed -e 's/^.*allows to \([^ ]* [^ ]*\) .*$/\1/' | sort -u
#git grep "allows$"

git checkout ChangeLog
