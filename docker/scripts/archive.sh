set -e

make archive_cc
ARCHIVE_NAME="$(ls temp_archive)"
ARCHIVE="$(ls temp_archive).tar.gz"
echo "$ARCHIVE_NAME"

tar -C temp_archive --no-same-owner -czvf "$ARCHIVE" "$ARCHIVE_NAME"
curl -f -H "X-JFrog-Art-Api:$ARTIFACTORY_API_KEY" -T "$ARCHIVE" "https://artifacts.corp.xperi.com/artifactory/perceive-hw/$ARCHIVE"
