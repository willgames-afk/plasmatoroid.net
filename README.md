# plasmatoroid.net
Source for my website https://plasmatoroid.net

## Setup
- Make sure NGINX is installed
- Replace `/etc/nginx/nginx.conf` with the provided `nginx.conf`

## Minecraft server setup
- I also host a minecraft server on plasmatoroid.net
- Make sure java is installed (I use `openjdk-jre-25-headless`)
- Download minecraft server software to `/srv/minecraft`
- Copy `minecraft.service` and `minecraft.socket` to `/etc/systemd/system/` so the server runs on startup

## Port usage
- Port 80: HTTP
- Port 443: HTTPS
- Port 25565: Minecraft

## License
This is licensed under GPLv3, see `LICENSE`

## TLC
TLC is a little content framwork I wrote for doing templates, markdown, etc.
It looks at everything in the `./source/` directory, processes some of it, and writes/copies it to `./build/`.

For clarity, I'll use the term "snippet" to refer to some incomplete HTML, and "template" to refer to an .TL file that accepts a snippet as input.

### .TL files
- Files that are HTML Templates end with .tl instead of .html
- If the first line of a `.tl` file is `use someTemplateName`, tl will use that template
for this page
- They are HTML, but tlc will preprocess <tl ...> tags

### TL Tags
- `<tl head>`- This file is a template. For pages that use this template, put stuff that goes in the `<head></head>` of the page here.
- `<tl content`- This file is a template. For pages that use this template, put the content of the page here.
- `<tl add>...</tl>`- This file is a snippet. The stuff inside this tag will get added to the head of the page.