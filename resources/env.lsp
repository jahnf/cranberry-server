<? 
-- luasp environment variable listing

function list_vars(name,e)
	?><li><b><u><?=name?></u></b> <i>(<?=type(e)?>)</i></li><ul><?
	for i,v in pairs(e) do
		if type(v) == 'table' then
			list_vars(i,v)
		else
			?><li><b><?=i?></b> <i>(<?=type(v)?>)</i> := <?=tostring(v)?></li><?
		end
	end
	?></ul><?
end

title='Lua scripting environment variables'
?>
<html>
<head>
	<title><?=title?></title>
</head>
<body>
	<h2><?=title?></h2>
	<p><?=_VERSION?></p>
	<ul>
<?
	list_vars( 'env', env ) 
?>
	</ul>
</body>
</html>
