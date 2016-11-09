// custom javascript for the standalone tips page

$(document).on('ready', function(evt) {

    var supports_html5_storage = function() {
        try {
            return 'localStorage' in window && window['localStorage'] !== null;
        } catch (e) {
            return false;
        }
    };

    // record which tab the user last clicked to keep that open
    $('#tabs a').on('click', function(evt) {
        if (supports_html5_storage()) {
            var now = new Date().getTime(); // store in milliseconds
            var target = $(evt.target);
            var cssId = target.attr('data-target');
            localStorage['lastOpenedTab'] = cssId;
            localStorage['lastOpenedTabTime'] = now;
        }
    });

    // if they already have a last opened category, open it up on page load
    if (supports_html5_storage() && localStorage['lastOpenedTab'] && localStorage['lastOpenedTabTime']) {
        var now = new Date().getTime();
        var then = parseInt(localStorage['lastOpenedTabTime'], 10);
        var threshold = 1000*60*10; // milliseconds to minutes
        if ((now - then) < threshold) {
            // expand category
            $('#tabs a[data-target="'+localStorage['lastOpenedTab']+'"]').tab('show');
        }
    }

    // record which category the user last clicked to keep that open on next
    // page load within a certain time frame, i.e. navigate back to find the
    // category they had just opened still open now.
    $('.accordion-group h3').on('click', function(evt) {
        if (supports_html5_storage()) {
            var now = new Date().getTime(); // store in milliseconds
            var $h3 = $(evt.target);
            var cssId = $h3.attr('data-target');
            localStorage['lastOpenedCategory'] = cssId;
            localStorage['lastOpenedCategoryTime'] = now;
        }
    });

    // if they already have a last opened category, open it up on page load
    if (supports_html5_storage() && localStorage['lastOpenedCategory'] && localStorage['lastOpenedCategoryTime']) {
        var now = new Date().getTime();
        var then = parseInt(localStorage['lastOpenedCategoryTime'], 10);
        var threshold = 1000*60*10; // milliseconds to minutes
        if ((now - then) < threshold) {
            // expand category
            $lastOpenedCategory = $(localStorage['lastOpenedCategory']);
            if ($lastOpenedCategory.is(':visible')) {
                $lastOpenedCategory.collapse('show');
            } else {
                $(document).on('shown', '[data-toggle="tab"]', function(e) {
                    if ($lastOpenedCategory.is(':visible')) {
                        $lastOpenedCategory.collapse('show');
                        $(this).off(e);
                    }
                })
            }
            
        }
    }
	
	// Muratet ---
	// Injection d'un menu pour revenir au menu principal
	if (!document.body.className.match("tips-home")){
		$( "body" ).append("<div>\n<a href=../home.html>\n<img style='float:left; vertical-align:middle;' src=../../static/images/tipbar/nav-left-arrow.png> &nbsp;Back to main help</a></div>");
	}
	// ---

    function reprotocol(hostname){
      // Update the protocol of the hostname to match the current document
      return document.location.protocol + hostname.substr(hostname.indexOf(':')+1)
    }

    var crossOriginInterface = new Scratch.Views.CrossOriginInterface({
        actor: window.parent,
        target_domain: reprotocol(Scratch.INIT_DATA.COI.TARGET_DOMAIN),
        allowed_origins: [reprotocol(Scratch.INIT_DATA.COI.TARGET_DOMAIN)]
    });

    crossOriginInterface.post('setModelPath', [window.location.origin + window.location.pathname], null);

});

function toggleVideos(on) {
    if(on) $('iframe[orig_src]').each(function() { $(this).attr('src', $(this).attr('orig_src')); });
    else $('iframe').each(function() { $(this).attr('orig_src', $(this).attr('src')); $(this).attr('src', ''); });
}

function getFromWindow(to_get) {
    to_get = to_get.split(".");
    obj = window;
    for (var i = 0; i < to_get.length; i++) {
        obj = obj[to_get[i]];
    }
    return obj;
}