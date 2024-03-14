# WGS UE PLUGIN
This Unreal Engine plugin provides a connection to WorldsBase. With it you can:
 - read/write from a database

# Installation
1. Place `wgs` folder into `Plugins` directory <br><br> <img src="./Resources/docs/screenshots/wgs-directory.png" width="700"> <br><br> 
# Example 1: Add Data to Table
In this example, we will add data to a table using the `Insert Data` node. Skip to the end of this section to see the full blueprint example.

1. Right click and search for wgs subsystem node <br><br> <img src="./Resources/docs/screenshots/subsystem.png" width="500"> <br><br>
2. Drag from subsystem node and search for desired WGS function. In this example, we will use the Insert Data function. <br><br> <img src="./Resources/docs/screenshots/insert-data.png" width="500"> <br><br>
3. Fill in the table name <br><br> <img src="./Resources/docs/screenshots/table-name.png" width="200"> <br><br>
4. Drag from Data Rows and select the `Make Array` node <br><br> <img src="./Resources/docs/screenshots/make-array.png" width="400"> <br><br>
5. Add a pin to the Make Array node for each column you want to edit in your table
6. Drag from each pin and select `Make DataRow` <br><br> <img src="./Resources/docs/screenshots/make-data-row.png" width="400"> ![]() <br><br>
7. Fill out column name and value for each data row <br><br> <img src="./Resources/docs/screenshots/fill-data-rows.png" width="400"> <br><br>
8. Drag from Insert Data node execution pin and search for `Bind Event to On Http Request Completed`. This will execute once our backend call is complete. <br><br>  <img src="./Resources/docs/screenshots/http-complete-bind.png" width="600"> <br><br>
9. Make the subsystem the target for the Bind Event node <br><br> <img src="./Resources/docs/screenshots/http-complete-bind-target.png" width="400"> <br><br>
10. Drag from the event pin on the bind event node and create a custom event <br><br> <img src="./Resources/docs/screenshots/custom-event.png" width="400"> <br><br>
11. Finally, do something with the result. Here we just print to console. For Insert Data, the result will be a confirmation string. <br><br> <img src="./Resources/docs/screenshots/print-result.png" width="400"> <br><br>

Here is the complete blueprint: <br><br>
<img src="./Resources/docs/screenshots/example-insert-data.png" width="900">

# Example 2: Read Data from Table

To read data from a table, the setup is similar to example 1, except you don't have to insert any data into the `Get Table` node. Just enter the table name. Here is what the blueprint will look like: <br><br>

<img src="./Resources/docs/screenshots/get-table.png" width="700">