# HomeGenie Panel notifications

Mini supports the same device registration API as HomeGenie Server program 580 over its normal HTTP and MQTT command routes. ESP32 and ESP8266 builds use the same push relay and application key as program 580.

Panel probes `HomeAutomation.HomeGenie/Config/Notifications.Unregister` without a token. Mini responds with `{"Status":"Error","Message":"Token missing"}`. Registration uses `Notifications.Register/<token>/<model>` and removal uses `Notifications.Unregister/<token>`. Encode each path segment. Repeating either operation is safe. Mini keeps up to eight phones in persistent storage.

To opt a module into activity notifications, set its `HomeGeniePanel.Notifications` parameter to `On` through `Modules.ParameterSet`; use `Off` to disable it. For example, to enable the first output of a module with address `1` in domain `HomeAutomation.HomeGenie`, call:

    /api/HomeAutomation.HomeGenie/Config/Modules.ParameterSet/HomeAutomation.HomeGenie/1/HomeGeniePanel.Notifications/On

Substitute the domain and address returned by `Modules.List`. Delivery covers level and shutter changes, lock and contact states, and active motion, alarm and object detection events. Outgoing HTTPS requires Wi-Fi and a synchronized clock. Notifications are best effort: up to four events wait for delivery in RAM; a reboot clears pending events but preserves subscriptions and module settings.

To replace the relay key, send a JSON object in the body of a POST to
`/api/HomeAutomation.HomeGenie/Config/Notifications.RelayKey`. For example,
`{"key":"replacement"}` selects a custom key and `{"key":""}` restores the
built-in key. Use a printable ASCII header value of up to 512 characters,
without leading or trailing spaces. The response reports success or failure.
The key is stored separately from module parameters and registered devices.
