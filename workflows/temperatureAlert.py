title = '🥵 It\'s getting hot outside.'
description = '🥵'
content = '🌬️ Turned on the fan for you'

email_data = {
    "header": title,
    "subheader": description,
    "content": content
}

data = parameter[1]
threshold = 25
temperature_values = [item['Temperature'] for item in data if 'Temperature' in item]

# take the last 30 seconds
if len(temperature_values) > 30:
    temperature_values = temperature_values[::-1][:30]
    temperature_values = [float(t) for t in temperature_values]
    temperature =  sum(temperature_values) / len(temperature_values)
    if temperature > threshold:
        output[1] = title
        output[2] = email_data