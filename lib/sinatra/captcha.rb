module Sinatra
  module CaptchaHelpers
    def captcha opts={}
      if opts[:ajax]
        self.class.captcha_handler.ajax_html opts[:id] || 'captcha_ajax'
      else
        self.class.captcha_handler.html opts[:id] || 'captcha'
      end
    end
    def captcha_correct?
      self.class.captcha_handler.validate params['captcha']['response'], params['captcha']['challenge']
    end
  end
  module Captcha
    def self.registered app
      app.helpers CaptchaHelpers

      app.set :captcha_ttl,     60 unless app.respond_to? :captcha_ttl
      app.set :captcha_level,   2  unless app.respond_to? :captcha_level
      app.set :captcha_handler, Image.new(app)

      app.get '/captcha/refresh' do
        content_type 'text/plain'
        app.captcha_handler.html('captcha', params['key'])
      end

      app.get '/captcha' do
        content_type 'text/plain'
        app.captcha_handler.html
      end

      app.get '/captcha/snippet/:id' do
        content_type 'text/plain'
        app.captcha_handler.html params[:id]
      end

      app.get '/captcha/:key' do
        expires 0, :no_cache
        content_type 'image/png'
        app.captcha_handler.image params[:key]
      end

    end

    # TODO simpliify
    class Image
      def initialize app
        @generator = FastCaptcha.new(nil, app.captcha_level)
        @ttl = app.captcha_ttl
      end

      def html id='captcha', key=nil
        challenge = @generator.generate @ttl, false
        snippet = <<-HTML
          <div id="#{id}">
            <img src="/captcha/#{challenge.key}">
            <input type="hidden" name="captcha[challenge]" value="#{challenge.key}">
            <input autocomplete="off" id="cr#{id}" name="captcha[response]" value="">
          </div>
        HTML
      end

      def ajax_html id='captcha_ajax'
        divid = id + '_div'
        snippet = <<-HTML
          <script type="text/javascript">
            $(document).ready(function() {
              $('##{id}_v').click(function() {
                load_captcha_#{id}();
                $(this).closest('form').find('input[type="submit"]').removeAttr('disabled');
                $('##{id}_r').click(function() {
                  load_captcha_#{id}();
                  $(this).closest('form').find('input[type="submit"]').removeAttr('disabled');
                  return false;
                });
                $(this).remove();
                return false;
              });
            });
            function load_captcha_#{id}() {
                $('##{id}_r').hide();
                $('##{id}').slideDown(250);
                $('##{id}').load('/captcha/snippet/#{divid}', function() {
                  $('##{id}').find('input[name="captcha[response]"]').focus();
                });
                $('##{id}').delay(#{@ttl*500}).slideUp(250, function() {
                  $(this).closest('form').find('input[type="submit"]').attr('disabled', 'disabled');
                  $('##{id}_r').show();
                });
                return false;
            }
          </script>
          <div class="captcha" style="display:none" id="#{id}"></div>
          <button id="#{id}_v" class="submit">Show Captcha</button>
          <button id="#{id}_r" class="submit" style="display:none">Refresh Captcha</button>
        HTML
      end

      def image key
        @generator.refresh key
      end

      def validate response, key
        @generator.validate key, response
      end
    end
  end
end
