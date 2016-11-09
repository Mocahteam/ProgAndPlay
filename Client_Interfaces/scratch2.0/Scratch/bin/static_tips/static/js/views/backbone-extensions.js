Backbone.View.prototype.close = function(options) {
  options = options || {};
  if (!options.persist) {
    this.remove();
  }

  this.unbind();
  this.undelegateEvents();

  if (this.onClose) {
    this.onClose();
  }
}

Backbone.View.prototype.open = function(options) {
    this.initialize();
    this.model.fetch();
    this.delegateEvents() 
},

Backbone.View.prototype._super = function(funcName) {
  var value = this.constructor.__super__[funcName];
  
  if (typeof value == 'function') {
    return value.apply(this, _.rest(arguments));
  } else {
    return value
  }
}
