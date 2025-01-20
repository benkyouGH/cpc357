title = '🌧️ It\'s raining heavily outside. Your plant is gonna drown🫠'
description = 'Heavy rain outside'
content = '⚠️ We turned on the roof for ya '

email_data = {
    "header": title,
    "subheader": description,
    "content": content
}

data = parameter[1]
raining_values = [item['Raining'] for item in data if 'Raining' in item]
# take the last 30 seconds
if len(raining_values) > 30:
    raining_values = raining_values[::-1][:30]

if "1" in raining_values:
    output[1] = title
    output[2] = email_data
