<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js'></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
<script type='text/javascript'>
    var table;
    var data;

    google.load('visualization', '1', {packages: ['table']});
    google.setOnLoadCallback(initChart);

    function displayData(sensorData) {

        if (data.getNumberOfRows()) {
            data.removeRows(0, 10);
        }
        data.addRows(sensorData);
        var dateFormatter = new google.visualization.DateFormat({pattern: 'dd/MM/yyyy HH:mm:ss'});
        dateFormatter.format(data, 0);
        table.draw(data, options);
    }

    function loadData() {

        var sensorData = [];
        var channel_id = window.parent.location.pathname.match(/\d+/g);
        $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/feed.json?results=10', function (data) {

            for (var index = 0; index < data.feeds.length; ++index) {
                var created_at = data.feeds[index].created_at;
                var master;
                var nodes;
                if (data.feeds[index]!=null){ // Check that there is some data in this feed
                    if (data.feeds[index].field1 !=null){
                        master = JSON.parse(data.feeds[index].field1);
                    }
                    if (data.feeds[index].field2 != null){
                        nodes = JSON.parse(data.feeds[index].field2);
                    }
                    // check the other fields for nodes
                    for (var i=4; i< Object.keys(data.feeds[index]).length; i++){
                        var fieldName = Object.keys(data.feeds[index])[i];
                        if (data.feeds[index][fieldName] != null){
                            var fieldValue = data.feeds[index][fieldName];
                            var newNodes = JSON.parse(fieldValue);
                            nodes = nodes.concat(newNodes);
                        }
                    }
                }

                var childIds = [];
                var childAddresses = [];
                if (nodes != null ) {
                    for (var i = 0; i < nodes.length; i++) {
                        childIds.push(nodes[i].id);
                    }
                    childIds = childIds.toString();

                    for (var i = 0; i < nodes.length; i++) {
                        childAddresses.push(nodes[i].add);
                    }
                    childAddresses = childAddresses.toString();
                }
                else{
                    childIds = "";
                    childAddresses = "";
                }
                sensorData.push([created_at !== null ? new Date(created_at) : null, master.id !== null ? master.id : null, master.address !== null ? master.address : null, childIds !== "" ? childIds : null, childAddresses !== "" ? childAddresses : null]);
            }
            ;
            displayData(sensorData);
        });
    }

    function initChart() {

        data = new google.visualization.DataTable();
        data.addColumn('datetime', 'Time');
        data.addColumn('string', 'Mast Id');
        data.addColumn('string', 'Mast Add');
        data.addColumn('string', 'Child Nodes');
        data.addColumn('string', 'Child Adds');

        table = new google.visualization.Table(document.getElementById('table_div'));
        options = {
            width: '100%', height: '100%',
            showRowNumber: true,
            sortColumn: 0,
            sortAscending: false
        };


        loadData();

        setInterval('loadData()', 15000);
    }
</script>