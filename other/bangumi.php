<?php

//本php用于隐藏api地址信息
$user_code=$_GET['code'];
//$type=$_GET['tp'];
$to_code=$_GET['nu'];
$state=$_GET['state'];
$if_ok=true;
if(empty($to_code)||empty($state)){
 	$if_ok=false;
}
//$url='bangumi_apply_auth.php?code='.$user_code.
//    '&to_code='.$to_code.'&state='.$state;
$url='{your redirect url}'.$user_code.
'&to_code='.$to_code.'&state='.$state;
//传参数到bangumi_apply_auth处理[这里不会显示到主页上]
if($if_ok)
	file_get_contents($url);
//跳转至其他页面
//原因是如果用户使用手机浏览器经常会二次请求
//html返回的页面
$error=<<<EOF
<html>
    <script language="javascript" type="text/javascript">
    window.location.href='https://bangumi.irisu.cc/error.html';
    </script>
    <body>
    	<?php
    		die();
    	?>
    </body>
</html>

EOF;

$success=<<<EOF
<html>
    <script language="javascript" type="text/javascript">
    window.location.href='https://bangumi.irisu.cc/bangumi.html';
    </script>
    <body>
    	<?php
    		die();
    	?>
    </body>
</html>

EOF;

if($if_ok){
	echo $success;
}else{
	echo $error;
}

?>