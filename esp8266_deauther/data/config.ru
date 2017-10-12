#
# this is a small ruby webserver for fast UI development
# make sure you have ruby and rack gem
#
# $ sudo gem install rack
#
# once installed, run this:
#
# $ rackup config.ru
# 
# and access the given port on localhost
#


require "net/http"
require "uri"

# app root
@root = File.expand_path(File.dirname(__FILE__))

run Proc.new { |env|
  request = Rack::Request.new(env)
  path = Rack::Utils.unescape(env['PATH_INFO'])
  index_file = @root + "#{path}/index.html"
  if File.exists?(index_file)
    # Return the index
    [200, {'Content-Type' => 'text/html'}, [File.read(index_file)]]
  else
    # Pass the request to the directory app
    Rack::Directory.new(@root).call(env)
  end
}
