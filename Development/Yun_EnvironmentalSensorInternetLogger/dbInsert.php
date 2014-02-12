<html>
 <head>
  <title>PHP Database Test</title>
 </head>
 <body>
   <?php
      $radio_mac = $_GET["radio_mac"];
      $type = $_GET["type"];
      $value = $_GET["value"];

      echo "radio_mac = ", $radio_mac, "\n";
      echo "type = ", $type, "\n";
      echo "value = ", $value, "\n";

      mysql_connect( "localhost", "fafohcom_yun", "Ardu1n0Yun" );
      mysql_select_db( "fafohcom_environmental" ) or die( mysql_error() );

      echo "Connected!\n";

      $sql = "INSERT INTO measurements (`radio_mac`, `type`, `value`) VALUES (\"" . $radio_mac . "\"," . $type . "," . $value . ");";

//      $queryStr = "INSERT INTO data values (\"" . $key . "\", \"" . $value . "\", NULL); ";
//      echo $queryStr, "\n";

      echo $sql, "\n";

      $result = mysql_query( $sql );
      if (!$result) {
          die('Invalid query: ' . mysql_error());
      }

   ?>
 </body>
</html>
