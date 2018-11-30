while True:
    try:
        sleep(seconds_between_photos)#time between photos stated as global var above
        date = datetime.now().strftime('%d-%m-%Y-%H:%M:%S')#outputting time/date stamps
        camera.annotate_text = date
        filename = '/BLANK_photos/' + date + '.jpg' #giving them location and format
        camera.capture('/var/www/html' + filename)
        url = hostname + filename #combine
        pusher.trigger('photos', 'new_photo', {'url': url})#keyword trigger remember for iOS
    except Exception as e:
        print ('Error is:',e)
