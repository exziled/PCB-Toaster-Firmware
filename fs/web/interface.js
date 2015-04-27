
// A helper function to validate the JSON string
function IsJSONString(str) {
	try {
		JSON.parse(str);
	} catch (e) {
		return false;
	}

	return true;
}

// Holds a pair for a point in the reflow profile
var ProfilePair = function(time, temp) {
	this.time = time;
	this.temp = temp;
}

function ProfilePairCompare(a, b) {
	if (parseInt(a.time) > parseInt(b.time))
		return -1;
	if (parseInt(a.time) < parseInt(b.time))
		return 1;
	return 0;
}

// The user's reflow profile
var Profile = function() {

	this.pairs = [];
	this.maxPosSlope = 3;
	this.maxNegSlope = 6;

	// Get the profile from the users local storage
	this.load = function() {
		if (localStorage.getItem('profile') && IsJSONString(localStorage.getItem('profile'))) {
			this.pairs = JSON.parse(localStorage.getItem('profile'));
			this.pairs.sort(ProfilePairCompare);
		}
	}

	// Save the user's current profile to local storage
	this.save = function() {
		if (this.pairs.length != 0) {
			localStorage.setItem('profile', JSON.stringify(this.pairs));
		}
	}

	// Remove a point from the profile, return true if found
	this.removePairAtTime = function(time) {

		// Find the selected pair
		for (var i = this.pairs.length - 1; i >= 0; i--) {
			if (parseInt(this.pairs[i].time) == parseInt(time)) {
				this.pairs.splice(i, 1);

				return true;
			}
		}

		return false;
	}

	// Add a point to the reflow profile
	this.addPair = function(time, temp) {
		// First remove any point at the specified time
		this.removePairAtTime(time);

		var newPair = new ProfilePair(time, temp);
		this.pairs.push(newPair);
	}

	// Get points usable for graphing
	this.getGraphPoints = function() {
		var points = [];

		if (this.pairs.length == 0) {
			return points;
		}

		// First sort the pairs by time
		this.pairs = this.pairs.sort(ProfilePairCompare);

		// Create an array of x,y points
		for (var i = this.pairs.length - 1; i >= 0; i--) {
			var pair = [];

			pair.push(parseInt(this.pairs[i].time));
			pair.push(parseInt(this.pairs[i].temp));
			points.push(pair);
		};

		return points;
	}

	// Send the profile to the server
	this.sendData = function() {
		var points = this.getGraphPoints();

		var message = "";

		$.each(this.getGraphPoints(), function(index, point) {
			message += point[0]+","+point[1]+",";
		});

		message = message.substring(0, message.length - 1);

		console.log(message);

		url = "/set_profile=";
		url = url.concat(message);
		$.get(url, function(data) {
			if (data == "test") {
				displayAlert('alert-success', '<em>SUCCESS:</em> The profile was sent to the toaster!');
			}

			console.log(data);
		});
	}
};

var updateProfile = (function(context, profile) {
	
	// Get values from HTML Form
	var getValues = (function(context, profile) {
		
		var time = $(context).parent().parent().find("#temp-control-time").val()
		
		var temp = $(context).parent().parent().find("#temp-control-temp").val()

		if (profile.pairs.length >= 19) {
			displayAlert('alert-danger', '<em>Error:</em> Too many points added to the profile.');
			return;
		} else if (!$.isNumeric(time) || !$.isNumeric(temp)) {
			displayAlert('alert-danger', '<em>Error:</em> Invalid input.');
			return;
		}

		profile.addPair(time, temp);
	});
	
	// Get new values from HTML return new array
	return function(context, profile) {
		getValues(context, profile);
		return profile.getGraphPoints();
	};
})();

function updateTable(profile) {
	// Re-print Values
	$(".temp-control-data").empty();
	$.each(profile.getGraphPoints(), function(index, point) {
		$(".temp-control-data").append("<tr class=\"temp-control-pair\"><td class=\"temp-control-value temp-pair-time\">" + 
			point[0] + "</td><td class=\"temp-control-value\">" + point[1] + 
			"</td><td><button type=\"button\" class=\"temp-control-remove btn btn-default btn-sm\"><span class=\"glyphicon glyphicon-minus\" aria-hidden=\"true\"></span></button></td></tr>");
	});
}

function displayAlert(type, message) {
	$("#alert").addClass();
	$("#alert").addClass("alert");
	$("#alert").addClass(type);
	$('#alert').html(message);
	$("#alert").stop().slideDown("slow").delay(3000).slideUp("slow");
}

function receiveMessage(message) {
	console.log(message);

	switch (message) {
		case 'started':
			$("#bake").prop('disabled', false).html("Stop Bake!");
			break;
		case 'done':
			$("#bake").html("Bake").prop('disabled', false);
	}
}

function readTemperature() {
	console.log("attempting to read temperature");

	$.get("/get_temp", function(data) {
		console.log(data);

		console.log("temerature read successfully");
	});
}

$(function() {
	var plot = $.plot("#placeholder", []);

	var reflowProfile = new Profile();
	reflowProfile.load();
	reflowProfile.addPair(0, 25);

	// Update The Plot
	if (reflowProfile.pairs.length != 0) {
		plot.setData([reflowProfile.getGraphPoints()]);
		plot.setupGrid();
		plot.draw();

		updateTable(reflowProfile);
	}

	// Connect to the server
	var server = new FancyWebSocket('ws://echo.websocket.org');

	// Make global temporarily for debugging
	Server = server;

	//Let the user know we're connected
	server.bind('open', function() {
		displayAlert('alert-success', "<em>Success:</em> A connection has been made with the toaster.");
	});

	//OH NOES! Disconnection occurred.
	server.bind('close', function(data) {
		displayAlert("alert-danger", "<em>Error:</em> Lost connection to the toaster.");
	});

	// Log any messages sent from server
	server.bind('message', receiveMessage);

	server.connect();

	// Add a funtion reack to a start bake
	$("#bake").click(function() {

		// If we have at least 2 points
		if (reflowProfile.getGraphPoints().length < 2) {
			displayAlert("alert-warning", "<em>Warning:</em> Please add additional points to the profile.");
		} else if ($("#bake").html() == "Stop Bake!") {
			server.send('message', 'done');
			$(this).prop('disabled', true);
		} else {
			reflowProfile.sendData();
			$(this).prop('disabled', true);
		}
	});

	// Remove a point from the profile
	$(".temp-control-tbl").on("click", ".temp-control-remove", function() {

		// Get the time so we can remove any existing points if they exist
		var time = $(this).parent().parent().find(".temp-pair-time")[0].innerText;

		reflowProfile.removePairAtTime(time);

		plot.setData([reflowProfile.getGraphPoints()]);
		plot.setupGrid();
		plot.draw();
		
		updateTable(reflowProfile);

		reflowProfile.save();
	});
	
	// Add a point to the profile
	$("#temp-control-add").click(function() {
		
		// Update The Plot
		plot.setData([updateProfile(this, reflowProfile)]);
		plot.setupGrid();
		plot.draw();
		
		updateTable(reflowProfile);

		reflowProfile.save();

		// Clear the text from the inputs
		$("#temp-control-time").val("").focus();
		$("#temp-control-temp").val("");
	});

	// Add a handler for the enter key being pressed
	$(document).keypress(function(e) {
		if ((e.which == 13) && $("#temp-control-temp").is(":focus")) {
			$("#temp-control-add").trigger("click");
		}
	});

	setInterval(readTemperature, 1000);
});