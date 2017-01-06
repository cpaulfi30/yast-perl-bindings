FROM yastdevel/cpp-tw
# Some tests are written in Ruby so the yast2-ruby-bindings package is needed...
RUN zypper --gpg-auto-import-keys --non-interactive in --no-recommends yast2-ruby-bindings
COPY . /tmp/sources

