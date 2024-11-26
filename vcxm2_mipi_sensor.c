
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/version.h>

#include <media/tegra_v4l2_camera.h>
#include <media/tegracam_core.h>
#include "vcxm2_mipi.h"

static const struct of_device_id sensor_of_match[] = {
	{ .compatible = "imago,mipi_sensor",},
	{ },
};
MODULE_DEVICE_TABLE(of, sensor_of_match);

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.use_single_read = true,
	.use_single_write = true,
};


static inline int sensor_read_reg(struct camera_common_data *s_data,
				u16 addr, u8 *val)
{
	int err = 0;
	u32 reg_val = 0;

	dev_dbg(s_data->dev, "%s\n", __func__);

	*val = reg_val & 0xFF;

	return err;
}

static int sensor_write_reg(struct camera_common_data *s_data,
				u16 addr, u8 val)
{
	int err = 0;
	
	dev_dbg(s_data->dev, "%s\n", __func__);
	
	return err;
}


static int sensor_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	return 0;
}

static struct tegracam_ctrl_ops sensor_ctrl_ops = {
	.set_group_hold = sensor_set_group_hold,
};

static int sensor_power_get(struct tegracam_device *tc_dev)
{
	dev_dbg(tc_dev->dev, "%s\n", __func__);

	return 0;
}

static int sensor_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;

	dev_dbg(tc_dev->dev, "%s\n", __func__);

	if (unlikely(!pw))
		return -EFAULT;

	return 0;
}

static struct camera_common_pdata *sensor_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct device_node *np = dev->of_node;
	struct camera_common_pdata *board_priv_pdata;
	const struct of_device_id *match;

	dev_dbg(dev, "%s\n", __func__);

	if (!np)
		return NULL;

	match = of_match_device(sensor_of_match, dev);
	if (!match) {
		dev_err(dev, "Failed to find matching dt id\n");
		return NULL;
	}

	board_priv_pdata = devm_kzalloc(dev,
					sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;

	return board_priv_pdata;
}

static int sensor_set_mode(struct tegracam_device *tc_dev)
{
	dev_dbg(tc_dev->dev, "%s\n", __func__);

	return 0;
}

static int sensor_start_streaming(struct tegracam_device *tc_dev)
{
	dev_dbg(tc_dev->dev, "%s\n", __func__);

	return 0;
}

static int sensor_stop_streaming(struct tegracam_device *tc_dev)
{
	dev_dbg(tc_dev->dev, "%s\n", __func__);

	return 0;
}

static const int sensor_30fps[] = {
	30,
};

enum {
	SENSOR_MODE_1920X1080_CROP_30FPS,
};

static const struct camera_common_frmfmt sensor_frmfmt[] = {
	{{4096, 3008}, sensor_30fps, 1, 0, SENSOR_MODE_1920X1080_CROP_30FPS},
};


static struct camera_common_sensor_ops sensor_common_ops = {
	.numfrmfmts = ARRAY_SIZE(sensor_frmfmt),
	.frmfmt_table = sensor_frmfmt,
	.write_reg = sensor_write_reg,
	.read_reg = sensor_read_reg,
	.parse_dt = sensor_parse_dt,
	.power_get = sensor_power_get,
	.power_put = sensor_power_put,
	.set_mode = sensor_set_mode,
	.start_streaming = sensor_start_streaming,
	.stop_streaming = sensor_stop_streaming,
};

static int sensor_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "%s\n", __func__);

	return 0;
}

static const struct v4l2_subdev_internal_ops sensor_subdev_internal_ops = {
	.open = sensor_open,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
static int sensor_probe(struct i2c_client *client)
#else
static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
#endif
{
	struct device *dev = &client->dev;
	struct tegracam_device *tc_dev;
	int err = 0;

	dev_dbg(dev, "probing mipi sensor\n");

	if (!IS_ENABLED(CONFIG_OF) || !client->dev.of_node)
		return -EINVAL;

	tc_dev = devm_kzalloc(dev,
			sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;

	tc_dev->client = client;
	tc_dev->dev = dev;
	strncpy(tc_dev->name, MODMODULENAME, sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &sensor_regmap_config;
	tc_dev->sensor_ops = &sensor_common_ops;
	tc_dev->v4l2sd_internal_ops = &sensor_subdev_internal_ops;
	tc_dev->tcctrl_ops = &sensor_ctrl_ops;

	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}
	tegracam_set_privdata(tc_dev, (void *)tc_dev);

	err = tegracam_v4l2subdev_register(tc_dev, true);
	if (err) {
		dev_err(dev, "tegra camera subdev registration failed\n");
		return err;
	}

	dev_dbg(dev, "MIPI sensor registered\n");

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0)
static int
sensor_remove(struct i2c_client *client)
#else
static void
sensor_remove(struct i2c_client *client)
#endif
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct tegracam_device *tc_dev;

	if (!s_data)
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0)
		return -EINVAL;
#else
		return;
#endif

	tc_dev = (struct tegracam_device *)s_data->priv;

	tegracam_v4l2subdev_unregister(tc_dev);
	tegracam_device_unregister(tc_dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0)
	return 0;
#endif
}

static const struct i2c_device_id sensor_id[] = {
	{ "imago-sensor", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, sensor_id);

static struct i2c_driver imago_sensor_driver = {
	.driver = {
		.name = MODMODULENAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sensor_of_match),
	},
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};

module_i2c_driver(imago_sensor_driver);

MODULE_AUTHOR("IMAGO Technologies GmbH");
MODULE_DESCRIPTION("VisionCam XM2 MIPI sensor driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(MODVERSION);
