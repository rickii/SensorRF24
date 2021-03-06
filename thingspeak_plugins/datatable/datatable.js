<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js'></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
<script type='text/javascript'>
    var table;
    var data;

    google.load('visualization', '1', {packages:['table']});
    google.setOnLoadCallback(initChart);

    function displayData(sensorData) {

        if (data.getNumberOfRows()){
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
        $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/feed.json?results=10', function(data) {
            for (var index = 0; index < data.feeds.length; ++index){
                var k = data.feeds[index];
                sensorData.push([k.created_at !== null ? new Date(k.created_at) : null, k.field3 !==null ? Number(k.field3) : null, k.field1 !== null ? Number(k.field1) : null, k.field2 !==null ? k.field2 : null, k.field5!== null ? k.field5 : null ]);
            };
            displayData(sensorData);
        });
    }

    function initChart() {

        data = new google.visualization.DataTable();
        data.addColumn('datetime', 'Time');
        data.addColumn('number', 'Temp');
        data.addColumn('number', 'Node Id');
        data.addColumn('string', 'IP');
        data.addColumn('string', 'Mesh Id');

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
