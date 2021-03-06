<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js'></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
<script type='text/javascript'>

    var chart;
    var charts;
    var data;

    google.load('visualization', '1', {packages:['gauge']});
    google.setOnLoadCallback(initChart);

    function displayData(point) {

        data.setValue(0, 0, '�C');
        data.setValue(0, 1, point);
        chart.draw(data, options);
    }

    function loadData() {

        // variable for the data point
        var p;
        var channel_id = window.parent.location.pathname.match(/\d+/g);
        $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/feed/last.json', function(data) {

            // get the data point
            p = data.field3;
            if (p)
            {
                p = Math.round(p * 10) / 10;
                displayData(p);
            }
        });
    }

    function initChart() {

        data = new google.visualization.DataTable();
        data.addColumn('string', 'Label');
        data.addColumn('number', 'Value');
        data.addRows(1);

        chart = new google.visualization.Gauge(document.getElementById('chart_div'));
        options = {
            width: 180, height: 180,
            min: -20, max: 40,
            greenFrom: -20, greenTo: 0, greenColor: '#616cff',
            redFrom: 30, redTo: 40,
            majorTicks: ['-20', '-10', '0', '10', '20', '30', '40'],
            minorTicks: 5
        };

        loadData();
        setInterval('loadData()', 15000);
    }


</script>