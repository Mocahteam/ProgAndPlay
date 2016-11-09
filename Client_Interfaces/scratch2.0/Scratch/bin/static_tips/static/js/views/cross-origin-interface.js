/*
  Simple API for cross origin communication when we are using iframes.
  For a given page, this needs to be initialized with the following options:
    1. An element that posts messages
    2. A list of allowed domains from which to recieve cross origin messages
    3. A list of allowed functions that can be run when received as callback
        functions from a received cross origin message
    5. The origin from which the message post comes.

 */

var Scratch = Scratch || {};
Scratch.Views = Scratch.Views || {};

Scratch.Views.CrossOriginInterface = Backbone.View.extend({

  defaults: {
    allowed_funcs: ['*'],
    allowed_origins: ['*'],
    target_domain: '*',
    context: window
  },

  initialize: function() {
    this.options = _.extend({}, this.defaults, this.options);
    window.addEventListener("message", $.proxy(this.messageListener, this), false);
  },

  functionAllowed: function(action) {
    return (this.options.allowed_funcs.indexOf(action) != -1) ||
           (this.options.allowed_funcs.indexOf('*') != -1);
  },

  originAllowed: function(domain) {
    return (this.options.allowed_origins.indexOf(domain) != -1);
  },

  //Called when 'message' events sent to window.
  messageListener: function(e) {
    if (!e.origin){
      e = e.originalEvent;
    }
    if (this.originAllowed(e.origin)) {
      var action_obj = $.parseJSON(e.data);
      if (this.functionAllowed(action_obj.action)) {
        this.receive(action_obj.action, action_obj.args, action_obj.callback);
      } else {
        throw "Received action not allowed";
      }
    }
  },

  //Runs received actions, if they were determined valid.
  receive: function(action, args, callback) {
    if (!action) return;
    var action_fn = this.options.context[action];
    var results = action_fn.apply(this.options.context, args);
    this.post(callback, [results], null);
  },

  //Posts message to actor
  post: function(action, args, callback) {
    if (!action) return;
    if (this.functionAllowed(action)) {
      var action_str = JSON.stringify({'action': action, 'args': args, 'callback': callback});
      this.options.actor.postMessage(action_str, this.options.target_domain);
    } else {
      throw "Posted action not allowed";
    }
  }
});
