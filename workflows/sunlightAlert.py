title = '🥀 Not enough sunlight'
description = 'Need sunlight...'
content = '💡 Turned on the lights'

email_data = {
    "header": title,
    "subheader": description,
    "content": content
}

data = parameter[1]
threshold = 500
brightness_values = [item['brightness'] for item in data if 'brightness' in item]
# take the last 30 seconds
if len(brightness_values) > 30:
    brightness_values = brightness_values[::-1][:30]
    brightness_values = [float(b) for b in brightness_values]
    brightness = sum(brightness_values) / len(brightness_values)
    if brightness < threshold:
        output[1] = title
        output[2] = email_data